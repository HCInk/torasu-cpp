#include "../include/torasu/std/EIcore_runner.hpp"

#include <sstream>
#include <memory>
#include <chrono>
#include <iostream>
#include <sstream> //for std::stringstream 
#include <stack> //for std::stack 

#include <torasu/log_tools.hpp>

using namespace std;

// Intercepts own log-group for every sub-task
#define INTERCEPT_LOGGER true

#define CHECK_STATE_ERRORS true
#define CHECK_REGISTRATION_ERRORS true
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
#define CACHE_MEM_MAX 1024*1024*1024

namespace torasu::tstd {

//
// EIcore_runner
//

EIcore_runner::EIcore_runner(bool useQueue, bool concurrent)
	: useQueue(useQueue), concurrentTree(concurrent), concurrentInterface(concurrent),
	  cache(new EIcore_runner_cacheinterface(CACHE_MEM_MAX)) {
#if INIT_DBG
	dbg_init();
#endif
}

EIcore_runner::EIcore_runner(size_t maxRunning)
	: threadCountMax(maxRunning), cache(new EIcore_runner_cacheinterface(CACHE_MEM_MAX)) {
#if INIT_DBG
	dbg_init();
#endif
	if (threadCountMax > 0) spawnThread(false);
}

EIcore_runner::~EIcore_runner() {
	stop();
#if INIT_DBG
	dbg_cleanup();
#endif
	delete cache;
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
		try {
			this->run(&threadHandle, collapse);
		} catch (const std::exception& ex) {
			std::cerr << "Runner thread crashed: " << ex.what() << std::endl;
		}
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

void EIcore_runner::run(EIcore_runner_thread* threadHandle, bool collapse) {
#if LOG_THREADS
	std::cout << " (THREAD) Enter thread " << std::to_address(threadHandle.thread) << std::endl;
#endif

	size_t retriesWithNone = 0;
	bool suspended = false;
	std::mutex threadWaiter;


#if CHECK_REGISTRATION_ERRORS
	{
		// Wait for registration...

		auto tid = std::this_thread::get_id();

#if LOG_THREADS
		std::cout << " (THREAD) Waiting for registration... " << tid << std::endl;
#endif

		for (;;) {
			{
				std::unique_lock lockedTM(threadMgmtLock);
				if (dbg->registered.find(tid) != dbg->registered.end()) break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

#if LOG_THREADS
		std::cout << " (THREAD) Found registration " << tid << std::endl;
#endif

	}
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

			if (!collapse || retriesWithNone < MAX_RETRIES) {
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

		if (suspended) { // Shutdown when still suspended
			break;
		}

		//
		// Get task
		//

		taskQueueLock.lock();

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

		taskQueueLock.unlock();

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
		threadHandle->running = false;
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
	if (concurrentInterface) taskQueueLock.lock();
	taskQueue.insert(obj);

	// std::cout << "TQ-insert " << obj << " (enqueue)" << std::endl;
	taskCv.notify_one();
	// std::cout << " ==== TASK QUEUE ====" << std::endl;
	// for (auto cObj : taskQueue) {
	// 	std::cout << "- ";
	// 	for (auto prio : *cObj->prioStack) {
	// 		std::cout << " " << prio;
	// 	}
	// 	std::cout << std::endl;
	// }

	if (concurrentInterface) taskQueueLock.unlock();



	return 0; // TODO Placeholder
}

EIcore_runner_object* EIcore_runner::createInterface(std::vector<int64_t>* prioStack) {
	int64_t newInterfaceId = interfaceIdCounter;
	interfaceIdCounter++;

	std::vector<int64_t>* selectedPrioStack;
	if (prioStack != NULL) {
		selectedPrioStack = prioStack;
	} else {
		selectedPrioStack = new std::vector<int64_t>();
	}

	// TODO create log-instruction for interface, dummy for now
	LogInstruction li(nullptr);
	return new EIcore_runner_object(this, newInterfaceId, li, selectedPrioStack);
}

// EIcore_runner: Housekeeping

EIcore_runner::Metrics EIcore_runner::getMetrics() {
	return {
		.queueSize = taskQueue.size(),
		.cacheItemCount = cache->getItemCount(),
		.cacheMemoryUsed = cache->getMemoryUsed(),
		.cacheMemoryMax = cache->getMemoryMax(),
	};
}

void EIcore_runner::clearCache() {
	cache->clearCache();
}

//
// EIcore_runner_object
//

inline void EIcore_runner_object::init() {
#if INTERCEPT_LOGGER
	if (li.logger != nullptr) {
		// Add interception-logger
		li.logger = new EIcore_runner_object_logger(this, li.logger);
		recordBench = li.options & torasu::LogInstruction::OPT_RUNNER_BENCH;
	}
#endif
}

// Subtask Constructor
EIcore_runner_object::EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, LogInstruction li, const std::vector<int64_t>* prioStack)
	: elemHandler(rnd != nullptr ? EIcore_runner_elemhandler::getHandler(rnd, runner) : nullptr),
	  rnd(rnd), li(li), parent(parent), runner(runner), renderId(renderId), prioStack(prioStack) {
	init();
}

// Interface Constructor
EIcore_runner_object::EIcore_runner_object(EIcore_runner* runner, int64_t renderId, LogInstruction li, const std::vector<int64_t>* prioStack)
	: elemHandler(nullptr), rnd(nullptr), li(li), parent(nullptr), runner(runner),
	  renderId(renderId), prioStack(prioStack), status(RUNNING) {
	init();
}

EIcore_runner_object::~EIcore_runner_object() {

	if (subTasks != nullptr) {
		bool log = li.level <= torasu::LogLevel::WARN;

		size_t foundSubTasks = 0;
		for (auto* subTask : *subTasks) {
			if (subTask != nullptr) foundSubTasks++;
		}

		if (foundSubTasks > 0) {

			if (log) li.logger->log(torasu::LogLevel::WARN, "Operation-exit, before all child-operation have been fetched! "
										"(" + std::to_string(foundSubTasks) + " still in queue) - will clean them up...");

			std::vector<EIcore_runner_object*> pendingSubTasks;
			for (auto* subTask : *subTasks) {
				if (subTask == nullptr) continue;
				int64_t rid = subTask->renderId;
				if (log) li.logger->log(torasu::LogLevel::WARN, " Cleaning up render of \"" + subTask->rnd->getType() + "\" (#" + std::to_string(rid) + ")...");
				std::unique_lock taskQueueLock(runner->taskQueueLock);
				std::unique_lock taskStatLock(subTask->statusLock);
				if (subTask->status == PENDING) {
					if (log) li.logger->log(torasu::LogLevel::WARN, " Freeing unstarted render of \"" + subTask->rnd->getType() + "\" (#" + std::to_string(rid) + ")...");
					runner->taskQueue.erase(subTask);
					taskQueueLock.unlock();
					delete subTask;
				} else {
					pendingSubTasks.push_back(subTask);
				}
			}

			for (auto* subTask : pendingSubTasks) {
				int64_t rid = subTask->renderId;
				if (log) li.logger->log(torasu::LogLevel::WARN, " Wating for uncompleted render of \"" + subTask->rnd->getType() + "\" (#" + std::to_string(rid) + ") to finish...");
				RenderResult* rr = fetchRenderResult(rid);
				delete rr;
			}

			if (log) li.logger->log(torasu::LogLevel::WARN, " Finished ending sub-operations - continue operation-exit.");

		}

		delete subTasks;

	}

	if (resultCv != nullptr) delete resultCv;
	if (resultCreation != nullptr) delete resultCreation;
	delete prioStack;
#if INTERCEPT_LOGGER
	if (li.logger != nullptr) delete li.logger; // Delete interception-logger
#endif
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

	status = SUSPENDED;

#if RUNNER_FULL_WAITS
	lockedStatus.unlock();
#endif
	while (status == SUSPENDED) {
		if (runner->threadCountRunning < runner->threadCountMax) {
			std::unique_lock threadLock(runner->threadMgmtLock);
			if (status == SUSPENDED && runner->threadCountRunning < runner->threadCountMax) {
				status = RUNNING;
				runner->registerRunning();
				return;
			}
		}
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

// EICoreRunnerObject: Benchmarking

void EIcore_runner_object::Benchmarking::init(LogInterface* logger, bool detailedLogging) {
	this->logger = logger;
	this->detailedLogging = detailedLogging;
	benchCalcSpent = 0;
	benchStart = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	benchRecentResume = benchStart;
}

void EIcore_runner_object::Benchmarking::resume() {
	benchRecentResume = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void EIcore_runner_object::Benchmarking::stop(bool final) {
	auto current = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	int64_t elapsedResume = current - benchRecentResume;
	if (elapsedResume < 0) elapsedResume = 0;
	benchCalcSpent += elapsedResume;

	if (logger != nullptr) {
		if (detailedLogging) {
			logger->log(LogBenchmark::createGroupBenchmark(elapsedResume, elapsedResume, benchRecentResume, !final));
		} else {
			if (final) {
				int64_t elapsedTotal = current - benchStart;
				if (elapsedTotal < 0) elapsedTotal = 0;
				logger->log(LogBenchmark::createGroupBenchmark(benchCalcSpent, elapsedTotal, benchStart));
			}
		}
	}
}

// EICoreRunnerObject: Execution Functions

RenderResult* EIcore_runner_object::run(std::function<void()>* outCleanupFunction) {

	torasu::tstd::EIcore_runner_elemhandler::ReadyStateHandle* rdyHandle;
	rdyHandle = elemHandler->ready(rs->getPipeline(), rctx, this, li);

	std::string addr;
	{
		std::stringstream ss;
		ss << this;
		addr = ss.str();
	}

	if (li.level <= LogLevel::TRACE) li.logger->log( LogLevel::TRACE, "(Runner) Task " + addr + " (" + rnd->getType() + ") Begin");

	if (recordBench) {
		bench.init(li.logger, li.options & torasu::LogInstruction::OPT_RUNNER_BENCH_DETAILED);
	}

	RenderInstruction ri(rctx, rs, this, li, rdyHandle != nullptr ? rdyHandle->state : nullptr);

	RenderResult* res;
	try {
		res = rnd->render(&ri);;
	} catch (const std::exception& ex) {
		torasu::tools::LogInfoRefBuilder lirb(li);
		lirb.logCause(torasu::ERROR, "Error rendering: " + std::string(ex.what()));
		res = new torasu::RenderResult(torasu::RenderResultStatus_INTERNAL_ERROR, lirb.build());
	}

	*outCleanupFunction = [rdyHandle]() {
		if (rdyHandle != nullptr) delete rdyHandle;
	};

	if (recordBench) bench.stop(true);

	if (li.level <= LogLevel::TRACE) li.logger->log( LogLevel::TRACE, "(Runner) Task " + addr + " (" + rnd->getType() + ") Finished");

	{
		auto* logger = static_cast<EIcore_runner_object_logger*>(li.logger);
		if (logger->registered) {
			auto* parentLogger = logger->logger;
			LogId logId = logger->ownLogId;
			// std::cout << "REREF..." << std::endl;
			std::vector<LogId> path({logId});
			res->reRefResult(parentLogger, &path,
			new Callback([parentLogger, logId] {
				// std::cout << "UNREF..." << std::endl;
				auto* unref = new torasu::LogEntry(LogType::LT_GROUP_UNREF);
				unref->groupStack.push_back(logId);
				parentLogger->log(unref);
			})
							);
		}

	}

	return res;
}

RenderResult* EIcore_runner_object::fetchOwnRenderResult() {

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

//
// torasu::ExecutionInterface implementation in EIcore_runner_object
//

uint64_t EIcore_runner_object::enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, LogInstruction li, int64_t prio) {

	bool lockSubTasks = parent != nullptr ? runner->concurrentSubCalls : runner->concurrentInterface;
	if (lockSubTasks) subTasksLock.lock();
	// Select renderId
	uint64_t newRenderId = renderIdCounter;
	renderIdCounter++;

	// Make prio-stack: {x,x,x,x,prio,id}
	auto* newPrioStack = new std::vector<int64_t>(*prioStack);
	newPrioStack->push_back(prio);
	newPrioStack->push_back(newRenderId);

	EIcore_runner_object* obj = new EIcore_runner_object(rnd, this, runner, newRenderId, li, newPrioStack);

	obj->setRenderContext(rctx);
	obj->setResultSettings(rs);

	if (rnd == nullptr) {
		obj->result = new RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
		obj->status = RUNNING;
	}

	auto stm = getSubTaskMemory(newRenderId);
	(*stm)[newRenderId] = obj;
	subTaskSize = newRenderId+1;

	if (lockSubTasks) subTasksLock.unlock();

	if (runner->useQueue && obj->status == PENDING) runner->enqueue(obj);

	return newRenderId;
}

void EIcore_runner_object::fetchRenderResults(ResultPair* requests, size_t requestCount) {
	if (recordBench) bench.stop();
	try {
		bool lockSubTasks = parent != nullptr ? runner->concurrentSubCalls : runner->concurrentInterface;
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
						std::unique_lock statLock(task->statusLock); // XXX This should be locked after queue
						if (task->status == PENDING) {

							// Run task if pending

							task->status = RUNNING;
							statLock.unlock();

							std::function<void()> cleanup;
							*fs.result = task->run(&cleanup);
							// ^ Result can be updated without locking since there are currently no other threads accessing this
							fs.task = nullptr;
							cleanup();

							if (runner->useQueue) { // Queue is not used in noQueue-mode
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

	} catch (const std::exception* ex) {
		if (recordBench) bench.resume();
		throw ex;
	}

	if (recordBench) bench.resume();

}

torasu::RenderResult* EIcore_runner_object::tryFetchRenderResult(uint64_t renderId) {
	bool lockSubTasks = parent != nullptr ? runner->concurrentSubCalls : runner->concurrentInterface;
	if (lockSubTasks) subTasksLock.lock();
	auto it = subTasks->begin()+renderId;
	EIcore_runner_object* task = *it;

	torasu::RenderResult* result = nullptr;
	if (task->result != nullptr) {
		std::unique_lock resultLocked(task->resultLock);
		result = task->result;
		if (result != nullptr) {
			*it = nullptr;
			delete task;
		}
	}

	if (lockSubTasks) subTasksLock.unlock();
	return result;
}

void EIcore_runner_object::lock(LockId lockId) {
	if (runner->concurrentTree) {
		if (recordBench) bench.stop();
		suspend();
		elemHandler->lock(lockId);
		unsuspend();
		if (recordBench) bench.resume();
	}
}

void EIcore_runner_object::unlock(LockId lockId) {
	if (runner->concurrentTree) {
		elemHandler->unlock(lockId);
	}
}

//
// EIcore_runner_object public diagnositc-functions
//

namespace {

std::string concatStrNTimes(std::string toRepeat, size_t n) {
	std::string repeatedStr;
	for (size_t i = 0; i < n; i++) {
		repeatedStr += toRepeat;
	}
	return repeatedStr;
}

} // namespace

void EIcore_runner_object::treestat(LogInstruction li, bool lock) {
	// TODO Operations executed in-fetch are not displayed yet
	bool doTreestatInfo = li.level <= torasu::INFO;
	// bool doTreestatWarn = li.level <= torasu::WARN;

	std::stack<std::stack<EIcore_runner_object*>> workingStack;
	std::stack<std::unique_lock<std::mutex>> lockStack;

	std::unique_lock<std::mutex> queueLck;
	if (lock) {
		queueLck = std::unique_lock<std::mutex>(runner->taskQueueLock);
	}

	std::stringstream ss;
	ss << "Overview over task-tree:\n";

	workingStack.emplace().push(this);

	while (!workingStack.empty()) {
		auto& currWs = workingStack.top();
		if (currWs.empty()) {
			workingStack.pop();
			if (lockStack.size() > workingStack.size())
				lockStack.pop();
			continue;
		}

		EIcore_runner_object* currObj = currWs.top();
		currWs.pop();

		if (lock) std::unique_lock objLck(currObj->statusLock);

		lockStack.emplace(currObj->subTasksLock);

		std::string name = currObj->rnd != nullptr ? currObj->rnd->getType().str : "(NULL)";
		std::string stateStr;
		switch(currObj->status) {
		case torasu::tstd::EIcore_runner_object_status::RUNNING: {
				stateStr = "RUNNING";
				break;
			}
		case torasu::tstd::EIcore_runner_object_status::SUSPENDED: {
				stateStr = "SUSPENDED";
				break;
			}
		case torasu::tstd::EIcore_runner_object_status::PENDING: {
				stateStr = "PENDING";
				break;
			}
		case torasu::tstd::EIcore_runner_object_status::BLOCKED: {
				stateStr = "BLOCKED";
				break;
			}
		default: {
				stateStr = "UNK:" + std::to_string(currObj->status);
			}
		}

		std::string infoLine = concatStrNTimes(" ", workingStack.size()) + " - " + name + " [" + stateStr + "]";
		ss << infoLine;

		if (currObj->subTasks != nullptr) {
			auto& subStack = workingStack.emplace();
			for (auto* subTask : *currObj->subTasks) {
				if (subTask != nullptr) subStack.push(subTask);
			}
			size_t total = currObj->subTaskSize;
			size_t active = subStack.size();
			if (total > 0) {
				ss << " {SUB: " << active << " ACTIVE / " << total <<  " TOTAL}";
			} else {
				ss << " {SUB: Created, but empoty}";
			}
		}

		ss << std::endl;

	}

	ss << "=== END OF TREESTAT ===";

	if (doTreestatInfo) {
		li.logger->log(INFO, ss.str());
	}

}

//
// EIcore_runner_object_logger
//

EIcore_runner_object_logger::EIcore_runner_object_logger(EIcore_runner_object* obj, LogInterface* logger)
	: obj(obj), logger(logger) {}

EIcore_runner_object_logger::~EIcore_runner_object_logger() {
	if (registered) {
		auto* uregEntry =
			new LogEntry(torasu::LogType::LT_GROUP_END);
		uregEntry->groupStack.push_back(ownLogId);
		logger->log(uregEntry);
	}
}

void EIcore_runner_object_logger::log(LogEntry* entry) {

	if (!registered) {
		std::unique_lock lock(groupLock);
		if (!registered) {
			ownLogId = logger->fetchSubId();
			auto* regEntry =
				new LogGroupStart(obj->rnd->getType().str);
			regEntry->groupStack.push_back(ownLogId);
			logger->log(regEntry);
			registered = true;
		}
	}

	entry->groupStack.push_back(ownLogId);

	logger->log(entry);
}

torasu::LogId EIcore_runner_object_logger::fetchSubId() {

	std::unique_lock lock(groupLock);
	auto subId = subIdCounter;
	subIdCounter++;
	return subId;

}

std::vector<LogId>* EIcore_runner_object_logger::pathFromParent(LogInterface* parent) const {
	if (parent == this) return new std::vector<LogId>(); // Found: parent is this

	if (!registered) return nullptr; // No-path: Path can't be generated to parent since this has no ID

	std::vector<LogId>* path = logger->pathFromParent(parent);

	if (path == nullptr) return nullptr; // Not found / No path

	path->push_back(ownLogId);

	return path; // Found path
}

//
// EIcore_runner_object_cmp
//

bool EIcore_runner_object_cmp::operator()(EIcore_runner_object* const& r, EIcore_runner_object* const& l) const {
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
// EIcore_runner_cacheinterface
//

EIcore_runner_cacheinterface::EIcore_runner_cacheinterface(int64_t maxMem)
	: maxMem(maxMem) {}

EIcore_runner_cacheinterface::~EIcore_runner_cacheinterface() {
	for (auto* handle : handles) {
		if (handle->tryDereference()) {
			delete handle;
		} else {
			std::cerr << "Got a no-dereference for a cached handle on freeing chache! - not freeing" << std::endl;
		}
	}
}

double EIcore_runner_cacheinterface::now() {
	double ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count();
	return ms / 1000;
}

bool EIcore_runner_cacheinterface::reserveSpace(int64_t space) {
	totalUsed += space;

	if (totalUsed <= maxMem) return true;

	std::multimap<double, CacheHandle*> ranked;

	double now = this->now() + 0.001;

	for (CacheHandle* handle : handles) {
		if (handle->inUse) continue;
		double existingFor = now - handle->genTimeStamp;
		double value = handle->calcTime * handle->hits / handle->size / existingFor;

		ranked.insert(std::pair<double,CacheHandle*>(value, handle));
	}

	for (auto rankedHandle : ranked) {
		CacheHandle* handle = rankedHandle.second;
		if (handle->tryDereference()) {
			totalUsed -= handle->size;

			handles.erase(handle);
			delete handle;

			if (totalUsed <= maxMem) return true;
		}
	}

	return false;
}

void EIcore_runner_cacheinterface::add(CacheHandle* handle) {
	std::unique_lock lck(opLock);

	reserveSpace(handle->size);

	handles.insert(handle);

}

void EIcore_runner_cacheinterface::remove(CacheHandle* handle) {
	std::unique_lock lck(opLock);

	totalUsed -= handle->size;

	handles.erase(handle);

}

void EIcore_runner_cacheinterface::clearCache() {
	std::set<CacheHandle*> removedHandles;
	for (auto* handle : handles) {
		if (handle->tryDereference()) {
			removedHandles.insert(handle);
		}
	}
	for (auto* removedHandle : removedHandles) {
		totalUsed -= removedHandle->size;
		handles.erase(removedHandle);
		delete removedHandle;
	}
}

size_t EIcore_runner_cacheinterface::getItemCount() {
	return handles.size();
}
size_t EIcore_runner_cacheinterface::getMemoryUsed() {
	return totalUsed;
}

size_t EIcore_runner_cacheinterface::getMemoryMax() {
	return maxMem;
}

//
// EIcore_runner_elemhandler
//

EIcore_runner_elemhandler::EIcore_runner_elemhandler(Element* elem, EIcore_runner* parent)
	: elem(elem), parent(parent), cache(parent->cache) {}

EIcore_runner_elemhandler::~EIcore_runner_elemhandler() {
	cleanReady();
}

class EIcore_runner_elemhandler::LoadedReadyState : public EIcore_runner_cacheinterface::CacheHandle {
private:
	std::mutex useLock;
	size_t uses;
	EIcore_runner_elemhandler* handler;

public:
	torasu::ReadyState* const rdys;

	LoadedReadyState(torasu::ReadyState* rdys, EIcore_runner_elemhandler* handler)
		: CacheHandle(-1, -1, 1, true), uses(1), handler(handler), rdys(rdys) {
		useLock.lock(); // Lock until finish(...) is called
		if (rdys->getContextMask() != nullptr) { // Only add if it has a valid mask
			// vs-code extension is kinda dumb there (tested on vscode-cpptools 1.2.2)
			// Cast required since it appreantly detects 'this' as 'LoadedReadyState' instead of 'EIcore_runner_elemhandler::LoadedReadyState'
			handler->readyStates.insert( (EIcore_runner_elemhandler::LoadedReadyState*) this);
		}
	}

	ReadyStateHandle* finish(double calcTime) {
		this->genTimeStamp = torasu::tstd::EIcore_runner_cacheinterface::now();
		this->calcTime = calcTime;
		this->size = rdys->size();
		handler->cache->add(this);
		useLock.unlock();

		// same thing with vs-code ext as above
		return new ReadyStateHandle( (EIcore_runner_elemhandler::LoadedReadyState*) this);
	}

	ReadyStateHandle* newUse() {
		std::unique_lock useLck(useLock);
		if (uses <= 0) inUse = true;
		uses++;
		hits++;

		// same thing with vs-code ext as above
		return new ReadyStateHandle( (EIcore_runner_elemhandler::LoadedReadyState*) this);
	}

	void unregUse() {
		std::unique_lock useLck(useLock);
		uses--;
		if (uses <= 0) inUse = false;
	}


	bool tryDereference() override {
		std::unique_lock listLck(handler->readyStatesLock);
		std::unique_lock useLck(useLock);
		if (inUse) return false;
		// same thing with vs-code ext-dumbness as above
		handler->readyStates.erase( (EIcore_runner_elemhandler::LoadedReadyState*) this);
		return true;
	}

	~LoadedReadyState() {
		delete rdys;
	}
};

EIcore_runner_elemhandler::ReadyStateHandle::ReadyStateHandle(LoadedReadyState* lrs)
	: lrs(lrs), state(lrs->rdys) {}

EIcore_runner_elemhandler::ReadyStateHandle::~ReadyStateHandle() {
	lrs->unregUse();
}

EIcore_runner_elemhandler::ReadyStateHandle* EIcore_runner_elemhandler::ready(torasu::Identifier operation, torasu::RenderContext* const rctx, EIcore_runner_object* obj, LogInstruction li) {
	std::unique_lock listLck(readyStatesLock);

	// Look for existing state in list

	for (auto* currentState : readyStates) {
		for (auto foundOp : *currentState->rdys->getOperations()) {
			if (foundOp == operation) {
				// Found macthing operation, use if rctx fits
				auto checkResult = currentState->rdys->getContextMask()->check(rctx);
				if (checkResult == torasu::DataResourceMask::MaskCompareResult::MCR_INSIDE) {
					// Operation and Rctx fits, add usage
					return currentState->newUse();
				}
			}
		}

	}

	// Create new ready-state

	LoadedReadyState* state = nullptr;

	class ReadyHandler : public torasu::ReadyInstruction {
	private:
		LoadedReadyState** stateOut;
		std::unique_lock<std::mutex>* listLock;
		EIcore_runner_elemhandler* elemHandler;
	public:
		ReadyHandler(Identifier operation, RenderContext* rctx, EIcore_runner_object* obj, LogInstruction li, LoadedReadyState** stateOut, std::unique_lock<std::mutex>* listLock, EIcore_runner_elemhandler* elemHandler)
			: ReadyInstruction(std::vector<Identifier>({operation}), rctx, obj, li), stateOut(stateOut), listLock(listLock), elemHandler(elemHandler) {}

		void setState(ReadyState* state) override {
			if (state != nullptr) {
				*stateOut = new EIcore_runner_elemhandler::LoadedReadyState(state, elemHandler);
			}
			listLock->unlock();
		}
	} readyHandler(operation, rctx, obj, li, &state, &listLck, this);

	bool previousDoBenchSetting = obj->recordBench;

	obj->recordBench = true;

	obj->bench.init(li.options & LogInstruction::OPT_RUNNER_BENCH ? li.logger : nullptr, li.options & LogInstruction::OPT_RUNNER_BENCH_DETAILED);

	try {
		elem->ready(&readyHandler);
	} catch (const std::exception& ex) {
		if (li.level <= ERROR) li.logger->log(ERROR, std::string("An error occurred while making object ready! - Message: ") + ex.what());
	}

	obj->bench.stop(true);
	obj->recordBench = previousDoBenchSetting;

	if (state != nullptr) {
		if (state->rdys->getContextMask() == nullptr)
			torasu::tools::log_checked(li, torasu::INFO, "Ready-State contains has no mask! This may lead to very reduced performance, since it can't be cached");

		// TODO Benchmark the result to calculate a non-dummy calcTime
		return state->finish(static_cast<double>(obj->bench.benchCalcSpent) / (1000*1000));
	}

	return nullptr;
}

void EIcore_runner_elemhandler::cleanReady() {
	for (LoadedReadyState* rdyState : readyStates) {
		cache->remove(rdyState);
		delete rdyState;
	}
}

/*

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

*/

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
		auto found = registered.find(tid);
		if (found != registered.end()) {
			if (found->second.second == true) {
				throw std::logic_error("Sanity-Check: " + RRN(found->second.first) + "-thread is already registered as running!");
			}
			found->second.second = true;
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

		threadCountRunning = threadCountRunning + 1;
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
		threadCountRunning = threadCountRunning - 1;
	}

}

} // namespace torasu::tstd