#include "../include/torasu/std/EIcore_runner.hpp"

#include <sstream>
#include <memory>
#include <chrono>
#include <iostream>

using namespace std;

#define CHECK_STATE_ERRORS false
#define CHECK_REGISTRATION_ERRORS false
#define LOG_REGISTRATIONS_RUNNER false
#define LOG_REGISTRATIONS_GUESTS false
#define LOG_REGISTRATIONS_RESURRECT false
#define LOG_REGISTRATIONS_INTERNAL false
#define RUNNER_FULL_WAITS false
#define CHECK_THREADS true
#define LOG_THREADS false
#define LOG_TIMING false

#if LOG_REGISTRATIONS_RUNNER || LOG_REGISTRATIONS_GUESTS || LOG_REGISTRATIONS_REVIVES || LOG_REGISTRATIONS_INTERNAL
#define CHECK_REGISTRATION_ERRORS true
#endif

#if CHECK_REGISTRATION_ERRORS
#define INIT_DBG true
#endif

#define MAX_RETRIES 50
#define RETRY_WAIT 10
#define CYCLE_BUMP_THRESHOLD 10

namespace torasu::tstd {

//
// EIcore_runner
//

EIcore_runner::EIcore_runner(bool concurrent) 
	: lockQueue(concurrent) {
#if INIT_DBG
	dbg_init();
#endif
}

EIcore_runner::EIcore_runner(size_t maxRunning) 
	: threadCountMax(maxRunning) {
#if INIT_DBG
	dbg_cleanup();
#endif
	spawnThread(false);
}

EIcore_runner::~EIcore_runner() {
	if (threadCountMax > 0) stop();
#if INIT_DBG
	dbg_cleanup();
#endif
}

// EIcore_runner: Registration Helpers

inline void EIcore_runner::registerRunning() {
#if CHECK_REGISTRATION_ERRORS
	dbg_registerRunning(std::this_thread::get_id());
#else
	threadCountRunning++;
#endif
}

inline void EIcore_runner::unregisterRunning() {
#if CHECK_REGISTRATION_ERRORS
	dbg_unregisterRunning(EIcore_runner_dbg::INTERNAL);
#else
	if (doRun) threadSuspensionCv.notify_one();
	threadCountRunning--;
#endif
}

inline bool EIcore_runner::requestNewThread(EIcore_runner_THREAD_REQUEST_MODE mode) {
	std::unique_lock lockedTM(threadMgmtLock);
	if (!doRun) {
		return false;
	}
	if (threadCountRunning < threadCountMax) {
		if (mode == NEW) {
			spawnThread(true); // spwan handles the registration itself
		} else {
			registerRunning();
			if (mode == UNSUSPEND) threadCountSuspended--;
		}
		return true;
	} else {
		if (mode == OR_SUSPEND) threadCountSuspended++;
		return false;
	}
}

// EIcore_runner: Thread management

void EIcore_runner::stop() {
#if LOG_THREADS
	std::cout << "(STOP) Waiting for TM-lock to stop..." << std::endl;
#endif
	{
		std::unique_lock lockedTM(threadMgmtLock);
#if LOG_THREADS
		std::cout << "(STOP) List of threads to stop" << std::endl;
		for (EIcore_runner_thread& thread : threads) {
			std::cout << "	" << thread.running << " - " << std::to_address(thread.thread) << std::endl;
		}
#endif
		doRun = false;
	}
#if LOG_THREADS
	std::cout << "(STOP) Stopping threads..." << std::endl;
#endif
	for (EIcore_runner_thread& thread : threads) {
#if LOG_THREADS
		std::cout << "(STOP) DEL TR " << std::to_address(thread.thread) << std::endl;
#endif
#if CHECK_THREADS
		if (!thread.thread->joinable()) {
			std::cerr << "(STOP) Trying to join an unjoinable thread! - this is a bug - skipping join of thread." << std::endl;
		} else {
			thread.thread->join();
		}
#else
		thread.thread->join();
#endif
		delete thread.thread;
	}
	threads.clear();
#if LOG_THREADS
	std::cout << "(STOP) Stopped threads." << std::endl;
#endif
}

void EIcore_runner::spawnThread(bool collapse) {
	threads.push_back({});
	EIcore_runner_thread& threadHandle = threads.back();
	threadHandle.thread = new std::thread([this, &threadHandle, collapse]() {
		this->run(threadHandle, collapse);
	});
#if LOG_THREADS
	std::cout << "(SPWAN) Create thread " << std::to_address(threadHandle.thread) << std::endl;
#endif

#if CHECK_REGISTRATION_ERRORS
	dbg_registerRunning(threadHandle.thread->get_id(), EIcore_runner_dbg::RUNNER);
#else
	registerRunning();
#endif

}

void EIcore_runner::cleanThreads() {
#if LOG_THREADS
	std::cout << "(CLEAN) Begin thread-cleanup..." << std::endl;
#endif


#if CHECK_REGISTRATION_ERRORS
	{ // Wait for registration...
		
		auto tid = std::this_thread::get_id();
		for (;;) {
			{
				std::unique_lock lockedTM(threadMgmtLock);
				if (dbg->registered.find(tid) != dbg->registered.end()) break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

	}
#endif

	bool found;
	do {
		found = false;
		for (auto it = threads.begin(); it != threads.end(); it++) {
			auto& thread = *it;
			if (!thread.running) {
#if LOG_THREADS
				std::cout << "(CLEAN) DEL TR " << std::to_address(thread.thread) << std::endl;
#endif
				thread.thread->join();
				delete thread.thread;
				threads.erase(it);
				// Re-search for thread to clean
				found = true;
				break;
			}
		}
	} while(found);
#if LOG_THREADS
	std::cout << "(CLEAN) Finished thread-cleanup." << std::endl;
#endif
}

// EIcore_runner: Thread-looop

void EIcore_runner::run(EIcore_runner_thread& threadHandle, bool collapse) {
	
	size_t retriesWithNone = 0;
	bool suspended = false;
  	std::mutex threadWaiter;

#if LOG_THREADS
	std::cout << " (THREAD) Enter thread " << std::to_address(threadHandle.thread) << std::endl;
#endif

	while (doRun) {

		//
		// Suspension-Management
		//

		while (suspended && doRun) {
			
			if (requestNewThread(UNSUSPEND)) {
				suspended = false;
				retriesWithNone = 0;
				break;
			}

			if ((!collapse || retriesWithNone < MAX_RETRIES)) {
#if !RUNNER_FULL_WAITS
					std::unique_lock<std::mutex> lck(threadWaiter);
					threadSuspensionCv.wait_for(lck, std::chrono::milliseconds(RETRY_WAIT));
					retriesWithNone++;
#endif
				continue;
			} else {
				break;
			}
			
		}

		//
		// Get task
		//

		if (lockQueue) taskQueueLock.lock();

		EIcore_runner_object* task = nullptr;

		for (auto* currTask : taskQueue) {
			auto& statusLock = currTask->statusLock;

			if (currTask->status == PENDING) {
				std::unique_lock lockedStatus(statusLock);
				if (currTask->status == PENDING) { 
					currTask->status = RUNNING;
					task = currTask;
					break;
				}
			} else if (currTask->status == SUSPENDED) {
				std::unique_lock lockedStatus(statusLock);
				if (currTask->status == SUSPENDED) { 

#if CHECK_REGISTRATION_ERRORS
					dbg_giveRes(currTask);
#endif

					// Unsuspend / Transfer run-privilege to target
					currTask->status = RUNNING;
					currTask->unsuspendCv.notify_all();
					lockedStatus.unlock();



					// Try to request a new run-privilege for current thread, if not suspend
					suspended = !requestNewThread(OR_SUSPEND);

					if (suspended) {
						break; 	
					}
				}
			}
		}
		
		if (lockQueue) taskQueueLock.unlock();

		if (task == nullptr) { // Wait for task if no task is available / has been suspended
			consecutiveFedCycles = 0;
			if ((!collapse || retriesWithNone < MAX_RETRIES)) {
				#if !RUNNER_FULL_WAITS
					if (!suspended) {
						std::unique_lock<std::mutex> lck(threadWaiter);
						taskCv.wait_for(lck, std::chrono::milliseconds(RETRY_WAIT));
					}
					retriesWithNone++;
				#endif
				continue;
			} else {
				break;
			}
		}

		retriesWithNone = 0;

		// Request new thread if thread is used enough.
		if (threadCountRunning < threadCountMax) {
			consecutiveFedCycles++;
			if (consecutiveFedCycles > CYCLE_BUMP_THRESHOLD*threadCountRunning) {
				requestNewThread(NEW);
				consecutiveFedCycles = 0;
			}
		} else {
			consecutiveFedCycles = 0;
		}

		//
		// Running the task
		//

		std::function<void(void)> runCleanup;
		RenderResult* result = task->run(&runCleanup);

		//
		// Saving its result and removing task from queue
		//
		{
			std::unique_lock lockedRes(task->resultLock);

			task->result = result;
#if LOG_TIMING
			task->resultCreation 
				= new auto(std::chrono::high_resolution_clock::now());
#endif
			// XXX shouldn't lockedRes be unlocked now?
			if (task->resultCv != nullptr) task->resultCv->notify_all();

			std::unique_lock lockedQueue(taskQueueLock);
			// std::cout << "TQ-erase " << task << " (thread)" << std::endl;
			auto found = taskQueue.find(task);
			taskQueue.erase(found);
		}

		runCleanup();

		//
		// Thread-cleanup routine
		//

		if (scheduleCleanThreads) {
			std::unique_lock lockedTM(threadMgmtLock);
			if (!doRun) { // Cancel if doRun=false - since threads may not be edited in that state
				threadMgmtLock.unlock();
				break;
			}
			if (scheduleCleanThreads) {
				scheduleCleanThreads = false;
				cleanThreads();
			}
		}
	}

	{
		std::unique_lock lockedTM(threadMgmtLock);
		threadHandle.running = false;
#if CHECK_REGISTRATION_ERRORS
		if (suspended) {
			dbg_unregisterRunning(EIcore_runner::EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED);
		} else {
			dbg_unregisterRunning(EIcore_runner::EIcore_runner_dbg::RUNNER);
		}
#else
		if (!suspended) unregisterRunning();
#endif
		scheduleCleanThreads = true;
	}
#if LOG_THREADS
	std::cout << "(THREAD) Stopped running " << std::to_address(threadHandle.thread) << std::endl;
#endif
}

// EIcore_runner: Runner-specifc interfacing

int32_t EIcore_runner::enqueue(EIcore_runner_object* obj) {
	if (lockQueue) taskQueueLock.lock();
	taskQueue.insert(obj);

	// std::cout << "TQ-insert " << obj << " (enqueue)" << std::endl;
	if (threadCountMax > 0) taskCv.notify_one();
	// std::cout << " ==== TASK QUEUE ====" << std::endl;
	// for (auto cObj : taskQueue) {
	// 	std::cout << "- ";
	// 	for (auto prio : *cObj->prioStack) {
	// 		std::cout << " " << prio;
	// 	}
	// 	std::cout << std::endl;
	// }

	if (lockQueue) taskQueueLock.unlock();



	return 0; // TODO Placeholder
}

ExecutionInterface* EIcore_runner::createInterface(std::vector<int64_t>* prioStack) {
	int64_t newInterfaceId = interfaceIdCounter;
	interfaceIdCounter++;

	std::vector<int64_t>* selectedPrioStack;
	if (prioStack != NULL) {
		selectedPrioStack = prioStack;
	} else {
		selectedPrioStack = new std::vector<int64_t>();
	}

	return new EIcore_runner_object(this, newInterfaceId, selectedPrioStack);
}

//
// EIcore_runner_object
//

// Subtask Constructor
EIcore_runner_object::EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, const std::vector<int64_t>* prioStack)
	: elemHandler(rnd != nullptr ? EIcore_runner_elemhandler::getHandler(rnd, runner) : nullptr), 
		rnd(rnd), parent(parent), runner(runner), renderId(renderId), prioStack(prioStack) {}

// Interface Constructor
EIcore_runner_object::EIcore_runner_object(EIcore_runner* runner, int64_t renderId, const std::vector<int64_t>* prioStack) 
	: elemHandler(nullptr), rnd(nullptr), parent(nullptr), runner(runner), 
		renderId(renderId), prioStack(prioStack), status(RUNNING) {}

EIcore_runner_object::~EIcore_runner_object() {
	if (subTasks != nullptr) delete subTasks;
	if (resultCv != nullptr) delete resultCv;
	if (resultCreation != nullptr) delete resultCreation;
	delete prioStack;
}

// EIcore_runner_object: Suspension Functions

inline void EIcore_runner_object::suspend() {
	std::unique_lock lockedTM(runner->threadMgmtLock);
	{
		std::unique_lock lockedStatus(statusLock);
#if CHECK_STATE_ERRORS
		if (status != RUNNING) 
			throw std::logic_error("suspend() can only be called in state " 
				+ std::to_string(RUNNING) + " (RUNNING), but it was called in " + std::to_string(status));
		if (parent == nullptr)
			throw std::logic_error("suspend() can never be called on an interface!");
#endif
		// By setting own state to BLOCKED/SUSPENDED the thread is nolonger effectivly running and will free a thread
		// In ornder to be set to RUNNING another thread has to suspend itself or request another thread-privilege 
		runner->unregisterRunning();
		status = BLOCKED;
	}
	
	if (runner->threadCountRunning <= 0 && runner->threadCountSuspended <= 0) { // Spawn thread if number of running threads reach a critical value
		runner->spawnThread(false);
	}
}

inline void EIcore_runner_object::unsuspend() {
	std::unique_lock lockedStatus(statusLock);
#if CHECK_STATE_ERRORS
	if (status != BLOCKED) 
		throw std::logic_error("unsuspend() can only be called in state " 
			+ std::to_string(BLOCKED) + " (BLOCKED), but it was called in " + std::to_string(status));
#endif
	std::unique_lock threadLock(runner->threadMgmtLock);
	if (runner->threadCountRunning < runner->threadCountMax) {
		status = RUNNING;
		runner->registerRunning();
		return;
	}
	threadLock.unlock();
	status = SUSPENDED;

#if RUNNER_FULL_WAITS
	lockedStatus.unlock();
#endif
	while (status == SUSPENDED) {
#if !RUNNER_FULL_WAITS
			unsuspendCv.wait_for(lockedStatus, std::chrono::milliseconds(1));
			// std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
#endif
	}

#if CHECK_REGISTRATION_ERRORS
	runner->dbg_recieveRes(this);
#endif
}

// EIcore_runner_object: Helper Functions

inline std::vector<EIcore_runner_object*>* EIcore_runner_object::getSubTaskMemory(size_t maxIndex) {
	if (subTasks == NULL) {
		subTasks = new std::vector<EIcore_runner_object*>(addAmmount);
	}
	while (subTasks->size() <= maxIndex) {
		subTasks->resize(subTasks->size() + addAmmount);
	}
	return subTasks;
}

inline void EIcore_runner_object::setRenderContext(RenderContext* rctx) {
	this->rctx = rctx;
}

inline void EIcore_runner_object::setResultSettings(ResultSettings* rs) {
	this->rs = rs;
}

// EICoreRunnerObject: Execution Functions

RenderResult* EIcore_runner_object::run(std::function<void()>* outCleanupFunction) {
	std::vector<std::string> ops;
	for (auto& rss : *rs) {
		ops.push_back(rss->getPipeline());
	}

	ReadyRequest rdyRequest(ops, rctx);
	ReadyObjects* rdyObjs = rnd->requestReady(rdyRequest);

	bool makeReady = rdyObjs != nullptr;

	if (makeReady) {
		elemHandler->readyElement(*rdyObjs, this);
	}

	RenderInstruction ri(rctx, rs, this);

	RenderResult* res = rnd->render(&ri);

	*outCleanupFunction = [this, makeReady, rdyObjs]() {
		if (makeReady) {
			elemHandler->unreadyElement(*rdyObjs);
			delete rdyObjs;
		}
	};

	return res;
}

RenderResult* EIcore_runner_object::fetchOwnRenderResult() {
	
	if (runner->threadCountMax <= 0) {
		// Singlethreaded just-in-time
		std::function<void()> cleanup;
		result = run(&cleanup); 
		cleanup();
		return result;
	} else {
		// Multithreaded, getting waiting for result to come in

		// 0 = Not set to sleep yet, 1=Set to sleep, 2=cant be set to sleep
		int suspendState = 0;

		while (true) {

			if (result != NULL) {
				std::unique_lock lockedResult(resultLock);
				if (result != NULL) {
#if LOG_TIMING
					auto found = std::chrono::high_resolution_clock::now();
					std::cout << "RES FETCH " << std::chrono::duration_cast<std::chrono::nanoseconds>(found - *resultCreation).count() << "ns" << std::endl;
					auto unsuspendStart = std::chrono::high_resolution_clock::now();
#endif
					if (suspendState == 1) parent->unsuspend();
#if LOG_TIMING
					auto unsuspendEnd = std::chrono::high_resolution_clock::now();
					std::cout << "UNSUSPEND " << std::chrono::duration_cast<std::chrono::nanoseconds>(unsuspendEnd - unsuspendStart).count() << "ns" << std::endl;
#endif
					return const_cast<RenderResult*>(result); // casting the volatile away
				}
			}
			if (suspendState == 0) {
				if (parent->parent != nullptr) { // Check if parent is not an interface
#if LOG_TIMING
					auto suspendStart = std::chrono::high_resolution_clock::now();
#endif
					parent->suspend(); // Suspend the parent, which is waiting for the result
#if LOG_TIMING
					auto suspendEnd = std::chrono::high_resolution_clock::now();
					std::cout << "SUSPEND " << std::chrono::duration_cast<std::chrono::nanoseconds>(suspendEnd - suspendStart).count() << "ns" << std::endl;
#endif
					suspendState = 1;
				} else {
					suspendState = 2;
				}
			}
			
			{
				#if !RUNNER_FULL_WAITS
					std::unique_lock lck(resultLock); // XXX shouldn't this be an indivdual lock?
					if (resultCv == nullptr) {
						resultCv = new std::condition_variable();
					}
					resultCv->wait_for(lck, std::chrono::milliseconds(1));
					// std::this_thread::sleep_for(std::chrono::milliseconds(1));	
				#endif
			}
		}

	}

}

//
// torasu::ExecutionInterface implementation in EIcore_runner_object
//

uint64_t EIcore_runner_object::enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio) {
	
	bool lockSubTasks = parent != nullptr ? runner->concurrentSubCalls : runner->lockQueue;
	if (lockSubTasks) subTasksLock.lock();
	// Select renderId
	uint64_t newRenderId = renderIdCounter;
	renderIdCounter++;

	// Make prio-stack: {x,x,x,x,prio,id}
	auto* newPrioStack = new std::vector<int64_t>(*prioStack);
	newPrioStack->push_back(prio);
	newPrioStack->push_back(newRenderId);

	EIcore_runner_object* obj = new EIcore_runner_object(rnd, this, runner, newRenderId, newPrioStack);

	obj->setRenderContext(rctx);
	obj->setResultSettings(rs);

	auto stm = getSubTaskMemory(newRenderId);
	(*stm)[newRenderId] = obj;
	subTaskSize = newRenderId+1;

	if (lockSubTasks) subTasksLock.unlock();

	if (runner->threadCountMax > 0) runner->enqueue(obj);

	return newRenderId;
}

void EIcore_runner_object::fetchRenderResults(ResultPair* requests, size_t requestCount) {
	bool lockSubTasks = parent != nullptr ? runner->concurrentSubCalls : runner->lockQueue;
	if (lockSubTasks) subTasksLock.lock();

	struct FetchSet {
		RenderResult** result;
		EIcore_runner_object* task;
	};

	std::vector<FetchSet> toFetch(requestCount);
	
#if CHECK_REGISTRATION_ERRORS
	// register guest-thread if called over interface (parent == nullptr)
	if (parent == nullptr) runner->dbg_registerRunning(EIcore_runner::EIcore_runner_dbg::GUEST);
#endif

	// Run not yet done jobs
	for (int reqi = requestCount-1; reqi >= 0; reqi--) {
		int64_t renderId = requests[reqi].renderId;

		if (subTasks != NULL && ((uint32_t)renderId) < subTaskSize) {

			auto it = subTasks->begin()+renderId;
			EIcore_runner_object* task = *it;
			*it = NULL;
			if (lockSubTasks) subTasksLock.unlock();


			if (task != NULL) {
				
				auto& fs = toFetch[requestCount-reqi-1]; 
				// ^ in reverse, so that the last expected tasks to finish will be waited first,
				// so less waits/suspends will be triggered

				fs.task = task;
				fs.result = &requests[reqi].result;

				if (task->status == PENDING) {
					std::unique_lock statLock(task->statusLock);
					if (task->status == PENDING) {

						// Run task if pending

						task->status = RUNNING;
						statLock.unlock();

						std::function<void()> cleanup;
						*fs.result = task->run(&cleanup);
						// ^ Result can be updated without locking since there are currently no other threads accessing this
						fs.task = nullptr;
						cleanup();

						if (runner->threadCountMax > 0) { // Queue is not used in 0-thread mode
							std::unique_lock lockedQueue(runner->taskQueueLock);
							auto& taskQueue = runner->taskQueue;
							// std::cout << "TQ-erase " << task << " (fetch-run)" << std::endl;
							auto found = taskQueue.find(task);
							if (found == taskQueue.end()) 
								throw std::logic_error("Sanity-Check: Couldn't find matching task in queue!");
							taskQueue.erase(found);
						}
						delete task;

					}
				}

			} else {
				std::ostringstream errMsg;
				errMsg << "The object the given render-id ("
					<< renderId
					<< "), is reffering to, has already been fetched and freed!";
				throw runtime_error(errMsg.str());
			}

		} else {
			if (lockSubTasks) subTasksLock.unlock();
			std::ostringstream errMsg;
			errMsg << "The given render-id ("
				<< renderId
				<< ") was never created!";
			throw runtime_error(errMsg.str());
		}

	}

#if CHECK_REGISTRATION_ERRORS
	if (parent == nullptr) runner->dbg_unregisterRunning(EIcore_runner::EIcore_runner_dbg::GUEST);
#endif

	// Fetch results
	for (auto& fs : toFetch) {
		if (fs.task == nullptr) continue; // Skip already fetched tasks
		*fs.result = fs.task->fetchOwnRenderResult();
		delete fs.task;
	}

}

void EIcore_runner_object::lock(LockId lockId) {
	if (runner->threadCountMax > 0) {
		suspend();
		elemHandler->lock(lockId);
		unsuspend();
	}
}

void EIcore_runner_object::unlock(LockId lockId) {
	if (runner->threadCountMax > 0) {
		elemHandler->unlock(lockId);
	}
}

//
// EIcore_runner_object_cmp
//

bool EIcore_runner_object_cmp::operator()(EIcore_runner_object*const& r, EIcore_runner_object*const& l) const {
	size_t itSizeR = r->prioStack->size();
	size_t itSizeL = l->prioStack->size();
	
	bool lLonger = itSizeL > itSizeR;
	size_t itSize = lLonger ? itSizeR : itSizeL;

	auto itPrioR = r->prioStack->begin();
	auto itPrioL = l->prioStack->begin();
	for (size_t i = 0; i < itSize; i++) {

		if (*itPrioR != *itPrioL) {
			if (*itPrioR < *itPrioL) { // The higher the value, the smaller the priority
				return true; // Order R before L 
			} else {
				return false; // Order L before R
			}
		}

		itPrioR++;
		itPrioL++;
	}

	return lLonger; // Order R before L, if the stack of L is longer 
}

//
// EIcore_runner_elemhandler
//

EIcore_runner_elemhandler::EIcore_runner_elemhandler(Element* elem, EIcore_runner* parent)
	: elem(elem), parent(parent) {}

EIcore_runner_elemhandler::~EIcore_runner_elemhandler() {}

void EIcore_runner_elemhandler::readyElement(const ReadyObjects& toReady, ExecutionInterface* ei) {

	// Index for what objects are
	//	LOADED: Already ready
	//	LOADING: Are currently being made ready
	// 	NOT_LOADED: Are not yet loaded

	ReadyObjects toWait;
	ReadyObjects toMakeReady;

	readyStatesLock.lock();
	for (ReadyObject rdyObj : toReady) {
		EIcore_runner_rdystate& rdyState = readyStates[rdyObj];
		rdyState.useCount++;

		switch (rdyState.loaded) {
		case LOADED:
			break;
		case NOT_LAODED:
			toMakeReady.push_back(rdyObj);
			rdyState.useCount = LOADING;
			break;
		case LOADING:
			toWait.push_back(rdyObj);
			break;
		}
	}
	readyStatesLock.unlock();

	// Make NOT_LOADED objects ready

	ReadyInstruction rdyInstr(toReady, ei);
	elem->ready(rdyInstr);

	// Notifiy about the newly made ready elements

	readyStatesLock.lock();
	for (ReadyObject rdyObj : toMakeReady) {
		readyStates[rdyObj].loaded = LOADED;
	}
	readyStatesLock.unlock();

	// Wait for the LOADING elements

	if (toWait.size() > 0) {
		auto waitIt = toWait.begin();
		EIcore_runner_rdystate* rdyState = nullptr;
		while (true) {
			readyStatesLock.lock();
			if (rdyState == nullptr) {
				rdyState = &readyStates[*waitIt];
			}

			bool loaded = rdyState->loaded == LOADED;

			readyStatesLock.unlock();
			if (loaded) {
				waitIt++;
				if (waitIt == toWait.end()) {
					break;
				}
			} else {
#if !RUNNER_FULL_WAITS
				std::this_thread::sleep_for(std::chrono::milliseconds(1)); // TODO Better solution to waiting for others to finish
#endif
			}
		}
	}

	// Finished waiting, desired ready-state is now achieved!

}

void EIcore_runner_elemhandler::unreadyElement(const ReadyObjects& toUnready) {

	ReadyObjects toMakeUnready;

	readyStatesLock.lock();

	for (ReadyObject rdyObj : toUnready) {
		EIcore_runner_rdystate& rdyState = readyStates[rdyObj];
		rdyState.useCount--;
		if (rdyState.useCount <= 0) {
			rdyState.loaded = NOT_LAODED;
			toMakeUnready.push_back(rdyObj);
		}
	}

	UnreadyInstruction uri(toMakeUnready);
	elem->unready(uri);

	readyStatesLock.unlock();
}

void EIcore_runner_elemhandler::lock(LockId lockId) {

	lockStatesLock.lock();
	auto& lock = lockStates[lockId];
	lockStatesLock.unlock();

	lock.lock();
}

void EIcore_runner_elemhandler::unlock(LockId lockId) {
	lockStatesLock.lock();
	auto& lock = lockStates[lockId];
	lockStatesLock.unlock();

	lock.unlock();

}

//
// Run-registration sanity-checing
//

void EIcore_runner::dbg_init() {
	dbg = new EIcore_runner_dbg();
}

void EIcore_runner::dbg_cleanup() {
	delete dbg;
}

std::string EIcore_runner::EIcore_runner_dbg::regReasonName(RegisterReason reason) {
	switch (reason) {
	case INTERNAL:
		return "INTERNAL";
	case RESURRECT:
		return "RESURRECT";
	case GUEST:
		return "GUEST";
	case RUNNER:
		return "RUNNER";
	case RUNNER_CLOSE_SUSPENDED:
		return "RUNNER_CLOSE_SUSPENDED";
	default:
		return "UNKOWN";
	}
}


void EIcore_runner::dbg_giveRes(EIcore_runner_object* obj) {
	std::unique_lock resLck(dbg->resLock);
	if (dbg->resMap.find(obj) != dbg->resMap.end()) {
		throw std::logic_error("Sanity-Check: Trying to ressurect a task, which still has another ressurection pending!");
	}
	auto tid = std::this_thread::get_id();
#if LOG_REGISTRATIONS_RESURRECT
	std::cout << "Res-creation by " << tid << " for " << std::to_address(obj) << std::endl;
#endif
	dbg->resMap[obj] = tid;
	dbg_unregisterRunning(EIcore_runner_dbg::RESURRECT);

}

void EIcore_runner::dbg_recieveRes(EIcore_runner_object* obj) {
	std::unique_lock resLck(dbg->resLock);
	auto found = dbg->resMap.find(obj);
	if (found == dbg->resMap.end()) {
		throw std::logic_error("Sanity-Check: Ressurection was recieved, but was never noted!");
	}
	auto tid = std::this_thread::get_id();
#if LOG_REGISTRATIONS_RESURRECT
	std::cout << "Ressurect by " << found->second << " to " << tid << " (" << std::to_address(obj) << ")" << std::endl;
#endif
	dbg->resMap.erase(found);
	dbg_registerRunning(tid, EIcore_runner_dbg::RESURRECT);
}

void EIcore_runner::dbg_registerRunning(std::thread::id tid, EIcore_runner_dbg::RegisterReason reason) {
	auto& RRN = EIcore_runner_dbg::regReasonName;
	auto& registered = dbg->registered;
	std::unique_lock regLck(dbg->registerLock);
	if (reason == EIcore_runner_dbg::GUEST || reason == EIcore_runner_dbg::RUNNER) {

		if (reason == EIcore_runner_dbg::GUEST) {
#if LOG_REGISTRATIONS_GUESTS
			std::cout << "Register-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		} else {
#if LOG_REGISTRATIONS_RUNNER
			std::cout << "Register-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		}

		if (registered.find(tid) != registered.end()) {
			throw std::logic_error("Sanity-Check: Trying to register a new " + RRN(reason) + "-thread, but the thread is already/still registeerd!");
		}
		registered[tid] = std::pair<EIcore_runner_dbg::RegisterReason, bool>(reason, true);

	} else {
#if LOG_REGISTRATIONS_INTERNAL
		std::cout << "Register-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		auto foundGuest = registered.find(tid);
		if (foundGuest != registered.end()) {
			if (foundGuest->second.second == true) {
				throw std::logic_error("Sanity-Check: " + RRN(foundGuest->second.first) + "-thread is already registered as running!");
			}
			foundGuest->second.second = true;
		} else {
			throw std::logic_error("Sanity-Check: " + RRN(reason) + "-register was called on an target, which is not registered!");
		}
	}

	if (reason == EIcore_runner_dbg::GUEST) {

		dbg->guestCount++;

	} else if (reason != EIcore_runner_dbg::RESURRECT) {
		if (threadCountRunning >= threadCountMax) {
			throw std::logic_error("Sanity-Check: Trying to register a thread, even if the maximum ammount of threads are already running!");
		}

		threadCountRunning++;
	}
}

void EIcore_runner::dbg_registerRunning(EIcore_runner_dbg::RegisterReason reason) {
	dbg_registerRunning(std::this_thread::get_id(), reason);
}

void EIcore_runner::dbg_unregisterRunning(EIcore_runner_dbg::RegisterReason reason) {
	auto& RRN = EIcore_runner_dbg::regReasonName;
	auto& registered = dbg->registered;
	std::unique_lock regLck(dbg->registerLock);
	auto tid = std::this_thread::get_id();
	std::cout << "Unregister-thread (" << RRN(reason) << ") " << tid << std::endl;
	
	auto found = registered.find(tid);
	if (reason == EIcore_runner_dbg::GUEST || reason == EIcore_runner_dbg::RUNNER || reason == EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED) {

		if (reason == EIcore_runner_dbg::GUEST) {
#if LOG_REGISTRATIONS_GUESTS
			std::cout << "Unregister-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		} else {
#if LOG_REGISTRATIONS_RUNNER
			std::cout << "Unregister-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		}

		if (found == registered.end()) {
			throw std::logic_error("Sanity-Check: Trying to unregister a thread, which is not registered!");
		}
		if (reason == EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED) {
			if (found->second.first != EIcore_runner_dbg::RUNNER) {
				throw std::logic_error("Sanity-Check: Trying to unregister a thread, with the wrong reason!"
					"(Registered as " + RRN(found->second.first) + " but was unregistered with" 
					+ RRN(EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED) + " which requires the RR " 
					+ RRN(EIcore_runner_dbg::RUNNER) + ")");
			}
			if (found->second.second == true) {
				throw std::logic_error("Sanity-Check: Thread has to be stopped for " + RRN(EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED) + "!");
			}
		} else {
			if (found->second.first != reason) {
				throw std::logic_error("Sanity-Check: Trying to unregister a thread, with the wrong reason!"
					"(Registered as " + RRN(found->second.first) + " but unregistered with " + RRN(found->second.first) + ")");
			}
			if (found->second.second == false) {
				throw std::logic_error("Sanity-Check: Trying to unregister a " + RRN(reason) + "thread, which is registered as not running!");
			}
		}
		registered.erase(found);

	} else {

#if LOG_REGISTRATIONS_INTERNAL
		std::cout << "Unregister-thread (" << RRN(reason) << ") " << tid << std::endl;
#endif
		if (found != registered.end()) {

			if (found->second.second == false) {
				throw std::logic_error("Sanity-Check: " + RRN(found->second.first) + "-thread is already registered as not running!");
			}
			found->second.second = false;

		} else {
			throw std::logic_error("Sanity-Check: Trying to unregister a thread, which is not registered!");
		}
	}
	if (reason == EIcore_runner_dbg::GUEST) {
		dbg->guestCount--;
	} else if (reason != EIcore_runner_dbg::RUNNER_CLOSE_SUSPENDED && reason != EIcore_runner_dbg::RESURRECT) {

		if (threadCountRunning+dbg->guestCount <= 0) {
			throw std::logic_error("Sanity-Check: Trying to unregister thread, even if no threads are running!");
		}

		if (doRun) threadSuspensionCv.notify_one();
		threadCountRunning--;
	}

}

} // namespace torasu::tstd