#include "../include/torasu/std/EIcore_runner.hpp"

#include <sstream>
#include <memory>
#include <chrono>
#include <iostream>

using namespace std;

#define TORASU_STD_CHECK_EICORERUNNER true
#define TORASU_STD_DBG_EICORERUNNER_THREAD_LOG false
#define TORASU_STD_DBG_EICORERUNNER_TIMING_LOG true

#define MAX_RETRIES 50
#define RETRY_WAIT 10
#define CYCLE_BUMP_THRESHOLD 10

namespace torasu::tstd {

//
// EICoreRunner
//

EIcore_runner::EIcore_runner(bool concurrent) 
	: lockQueue(concurrent) {}

EIcore_runner::EIcore_runner(size_t maxRunning) 
	: threadCountMax(maxRunning) {
	spawnThread(false);
}

EIcore_runner::~EIcore_runner() {
	if (threadCountMax > 0) stop();
}

void EIcore_runner::stop() {
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(STOP) Waiting for TM-lock to stop..." << std::endl;
#endif
	{
		std::unique_lock lockedTM(threadMgmtLock);
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
		std::cout << "(STOP) List of threads to stop" << std::endl;
		for (EIcore_runner_thread& thread : threads) {
			std::cout << "	" << thread.running << " - " << std::to_address(thread.thread) << std::endl;
		}
#endif
		doRun = false;
	}
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(STOP) Stopping threads..." << std::endl;
#endif
	for (EIcore_runner_thread& thread : threads) {
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
		std::cout << "(STOP) DEL TR " << std::to_address(thread.thread) << std::endl;
#endif
#if TORASU_STD_CHECK_EICORERUNNER
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
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(STOP) Stopped threads." << std::endl;
#endif
}

void EIcore_runner::spawnThread(bool collapse) {
	threads.push_back({});
	EIcore_runner_thread& threadHandle = threads.back();
	threadHandle.thread = new std::thread([this, &threadHandle, collapse]() {
		this->run(threadHandle, collapse);
	});
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(SPWAN) Create thread " << std::to_address(threadHandle.thread) << std::endl;
#endif
	registerRunning();
}

void EIcore_runner::cleanThreads() {
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(CLEAN) Begin thread-cleanup..." << std::endl;
#endif
	bool found;
	do {
		found = false;
		for (auto it = threads.begin(); it != threads.end(); it++) {
			auto& thread = *it;
			if (!thread.running) {
				std::cout << "(CLEAN) DEL TR " << std::to_address(thread.thread) << std::endl;
				thread.thread->join();
				delete thread.thread;
				threads.erase(it);
				// Re-search for thread to clean
				found = true;
				break;
			}
		}
	} while(found);
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(CLEAN) Finished thread-cleanup." << std::endl;
#endif
}

void EIcore_runner::run(EIcore_runner_thread& threadHandle, bool collapse) {
	
	size_t retriesWithNone = 0;
	bool suspended = false;
  	std::mutex threadWaiter;

#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
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
#if !TORASU_TSTD_CORE_RUNNER_FULL_WAITS
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
				#if !TORASU_TSTD_CORE_RUNNER_FULL_WAITS
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
				std::unique_lock lockedTM(threadMgmtLock);
				if (threadCountRunning < threadCountMax && consecutiveFedCycles > CYCLE_BUMP_THRESHOLD*threadCountRunning) {
					spawnThread(NEW);
					consecutiveFedCycles = 0;
				}
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
#if TORASU_STD_DBG_EICORERUNNER_TIMING_LOG
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
		if (!suspended) unregisterRunning();
		scheduleCleanThreads = true;
	}
#if TORASU_STD_DBG_EICORERUNNER_THREAD_LOG
	std::cout << "(THREAD) Stopped running " << std::to_address(threadHandle.thread) << std::endl;
#endif
}

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
// EICoreRunnerObject
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
#if TORASU_STD_DBG_EICORERUNNER_TIMING_LOG
					auto found = std::chrono::high_resolution_clock::now();
					std::cout << "RES FETCH " << std::chrono::duration_cast<std::chrono::nanoseconds>(found - *resultCreation).count() << "ns" << std::endl;
					auto unsuspendStart = std::chrono::high_resolution_clock::now();
#endif
					if (suspendState == 1) parent->unsuspend();
#if TORASU_STD_DBG_EICORERUNNER_TIMING_LOG
					auto unsuspendEnd = std::chrono::high_resolution_clock::now();
					std::cout << "UNSUSPEND " << std::chrono::duration_cast<std::chrono::nanoseconds>(unsuspendEnd - unsuspendStart).count() << "ns" << std::endl;
#endif
					return const_cast<RenderResult*>(result); // casting the volatile away
				}
			}
			if (suspendState == 0) {
				if (parent->parent != nullptr) { // Check if parent is not an interface
#if TORASU_STD_DBG_EICORERUNNER_TIMING_LOG
					auto suspendStart = std::chrono::high_resolution_clock::now();
#endif
					parent->suspend();
#if TORASU_STD_DBG_EICORERUNNER_TIMING_LOG
					auto suspendEnd = std::chrono::high_resolution_clock::now();
					std::cout << "SUSPEND " << std::chrono::duration_cast<std::chrono::nanoseconds>(suspendEnd - suspendStart).count() << "ns" << std::endl;
#endif
					suspendState = 1;
				} else {
					suspendState = 2;
				}
			}
			
			{
				#if !TORASU_TSTD_CORE_RUNNER_FULL_WAITS
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
// Execution Interface
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
#if !TORASU_TSTD_CORE_RUNNER_FULL_WAITS
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

} // namespace torasu::tstd