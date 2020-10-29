#ifndef STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_
#define STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_

#include <vector>
#include <list>
#include <set>
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

#include <torasu/torasu.hpp>

#define TORASU_TSTD_CHECK_STATE_ERRORS true

namespace torasu::tstd {

class EIcore_runner_object;
class EIcore_runner_elemhandler;

struct EIcore_runner_object_cmp {
	bool operator()(EIcore_runner_object*const& r, EIcore_runner_object*const& l) const;
};

struct EIcore_runner_thread {
	std::thread* thread = nullptr;
	bool running = true;
};

enum EIcore_runner_THREAD_REQUEST_MODE {
	NEW, // Request a new thread
	UNSUSPEND, // From suspension to unsuspend
	OR_SUSPEND // After transfering run-privilege, will be set to suspended if not
};

class EIcore_runner {
protected:
	// Task-queue stuff (locked by taskQueueLock)
	std::mutex taskQueueLock;
	std::set<EIcore_runner_object*, EIcore_runner_object_cmp> taskQueue;
	std::condition_variable taskCv; // notify-one once task queue gets updated 

	// Thread-management (locked via threadMgmtLock)
	std::mutex threadMgmtLock;
	volatile bool doRun = true;
	size_t threadCountCurrent = 0; // The count of the threads, which are currently effectively running
	std::condition_variable threadSuspensionCv; // notify-one once another thread will be freed 
	size_t threadCountSuspended = 0; // The count of threads that are currently waiting to be reactivated
	size_t threadCountMax = 1;
	size_t consecutiveFedCycles = 0; // Consecutive cycles without task shortage
	std::list<EIcore_runner_thread> threads; // !!! Never edit if doRun=false
	volatile bool scheduleCleanThreads = false;
	inline void registerRunning() {
		threadCountCurrent++;
	}
	inline void unregisterRunning() {
		if (doRun) threadSuspensionCv.notify_one();
		threadCountCurrent--;
	}
	void cleanThreads();
	void spawnThread(bool collapse);
	
	// Interface creation-counter (not thread-safe)
	int64_t interfaceIdCounter = 0;

	// Internal functions (not thread-safe)
	void stop();

	// Internal functions (thread-safe)
	void run(EIcore_runner_thread& threadHandle, bool collapse);
	int32_t enqueue(EIcore_runner_object* obj);

	// Thread-management-tools (thread-safe / autolocking threadMgmtLock)
	inline bool requestNewThread(EIcore_runner_THREAD_REQUEST_MODE mode=NEW) {
		threadMgmtLock.lock();
		if (!doRun) {
			threadMgmtLock.unlock();
			return false;
		}
		if (threadCountCurrent < threadCountMax) {
			registerRunning();
			if (mode == UNSUSPEND) threadCountSuspended--;
			threadMgmtLock.unlock();
			return true;
		} else {
			if (mode == OR_SUSPEND) threadCountSuspended++;
			threadMgmtLock.unlock();
			return false;
		}
	}
public:
	EIcore_runner();
	~EIcore_runner();

	ExecutionInterface* createInterface(std::vector<int64_t>* prioStack=NULL);

	friend class EIcore_runner_object;
};

enum EIcore_runner_object_status {
	PENDING,// Needs executor
	RUNNING, // Currently running
	BLOCKED, // Currently blocked, shouldn't be resumed
	SUSPENDED // Currently suspended, waiting for resume
};

class EIcore_runner_object : public ExecutionInterface {
private:
	// Settings
	const size_t addAmmount = 10;
	
	// Object-data: Element Handler (persistent)
	EIcore_runner_elemhandler* elemHandler;

	// Object-data: Task-Information (persistent)
	Renderable* rnd;
	RenderContext* rctx = NULL;
	ResultSettings* rs = NULL;

	// Object-data: Task-Settings (persistent)
	int64_t renderId;
	EIcore_runner_object* parent;
	EIcore_runner* runner;
	std::vector<int64_t>* prioStack;


	// Sub-task-data (locked by "subTasksLock")
	std::mutex subTasksLock;
	int64_t renderIdCounter = 0;
	std::vector<EIcore_runner_object*>* subTasks = NULL;
	uint32_t subTaskSize = 0;

	// Own results (locked by "resultLock")
	std::mutex resultLock;
	volatile RenderResult* result = nullptr;
	std::condition_variable* resultCv = nullptr;

	// Progress status (locked by statusLock)
	std::mutex statusLock; // Locks status
	volatile EIcore_runner_object_status status = PENDING; // Status of task
	std::condition_variable unsuspendCv;

	inline void suspend() {
		runner->threadMgmtLock.lock();
		statusLock.lock();
#if TORASU_TSTD_CHECK_STATE_ERRORS
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
		statusLock.unlock();
		
		if (runner->threadCountCurrent <= 0 && runner->threadCountSuspended <= 0) { // Spawn thread if number of running threads reach a critical value
			runner->spawnThread(false);
		}
		runner->threadMgmtLock.unlock();
	}

	inline void unsuspend() {
		std::unique_lock lck(statusLock);
#if TORASU_TSTD_CHECK_STATE_ERRORS
		if (status != BLOCKED) 
			throw std::logic_error("unsuspend() can only be called in state " 
				+ std::to_string(BLOCKED) + " (BLOCKED), but it was called in " + std::to_string(status));
#endif
		std::unique_lock threadLock(runner->threadMgmtLock);
		if (runner->threadCountCurrent < runner->threadCountMax) {
			status = RUNNING;
			runner->registerRunning();
			return;
		}
		threadLock.unlock();
		status = SUSPENDED;
		lck.unlock();
		while (status == SUSPENDED) {
			// unsuspendCv.wait_for(lck, std::chrono::milliseconds(1)); // XXX Removed for performance-optimisation-testing (186 -> 190)
			// std::this_thread::sleep_for(std::chrono::milliseconds(1)); // XXX Removed for performance-optimisation-testing
		}
	}

	inline std::vector<EIcore_runner_object*>* getSubTaskMemory(size_t maxIndex) {
		if (subTasks == NULL) {
			subTasks = new std::vector<EIcore_runner_object*>(addAmmount);
		}
		while (subTasks->size() <= maxIndex) {
			subTasks->resize(subTasks->size() + addAmmount);
		}
		return subTasks;
	}

protected:
	EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, std::vector<int64_t>* prioStack);
	EIcore_runner_object(EIcore_runner* runner, int64_t renderId, std::vector<int64_t>* prioStack);
	virtual ~EIcore_runner_object();

	RenderResult* run(std::function<void()>* outCleanupFunction);
	RenderResult* fetchOwnRenderResult();

	inline void setRenderContext(RenderContext* rctx) {
		this->rctx = rctx;
	}

	inline void setResultSettings(ResultSettings* rs) {
		this->rs = rs;
	}

public:
	uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio) override;
	RenderResult* fetchRenderResult(uint64_t renderId) override;
	void lock(LockId lockId) override;
	void unlock(LockId lockId) override;

	friend class EIcore_runner;
	friend struct EIcore_runner_object_cmp;
};

enum EIcore_runner_rdystate_LOADSTATE {
	NOT_LAODED,
	LOADING,
	LOADED
};

class EIcore_runner_rdystate {
public:
	uint64_t useCount = 0;
	EIcore_runner_rdystate_LOADSTATE loaded = NOT_LAODED;
};

class EIcore_runner_elemhandler : public ElementExecutionOpaque {
private:
	// Information about the element
	Element* elem;
 	EIcore_runner* parent;

	// Ready-States
	std::mutex readyStatesLock; // Lock for readyStates and its contents
	std::map<ReadyObject, EIcore_runner_rdystate> readyStates;

	// Locks
	std::mutex lockStatesLock; // Lock for lockStates and its contents
	std::map<LockId, std::mutex> lockStates;

public:
	static inline EIcore_runner_elemhandler* getHandler(Element* elem, EIcore_runner* parent) {
		elem->elementExecutionOpaqueLock.lock();
		if (elem->elementExecutionOpaque == nullptr) {
			elem->elementExecutionOpaque = new EIcore_runner_elemhandler(elem, parent);
		}
		elem->elementExecutionOpaqueLock.unlock();
		return reinterpret_cast<EIcore_runner_elemhandler*>(elem->elementExecutionOpaque);
	}

	EIcore_runner_elemhandler(Element* elem, EIcore_runner* parent);
	~EIcore_runner_elemhandler();

	void readyElement(const ReadyObjects& toReady, ExecutionInterface* ei);
	void unreadyElement(const ReadyObjects& toUnready);
	void lock(uint64_t lockId);
	void unlock(uint64_t lockId);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_
