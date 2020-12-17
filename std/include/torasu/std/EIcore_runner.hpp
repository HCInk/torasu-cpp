#ifndef STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_
#define STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_

#include <stdlib.h>

#include <vector>
#include <list>
#include <set>
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <string>
#include <utility>

#include <torasu/torasu.hpp>


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
	// Weather the central queue should be used
	const bool useQueue = true;
	// Determins weather the tree has to be thread-safe - set to true if multiple threads are running the same tree
	const bool concurrentTree = true;
	// Determins weather interface exposed to managed tasks needs to be concurrent
	const bool concurrentInterface = true;
	// Determins weather the queueing/fetching of tasks inside the runner has to be thread-safe 
	const bool concurrentSubCalls = true;
	// The maximum ammount of threads to be run by the 
	int64_t threadCountMax = 0;

	// Task-queue stuff (locked by taskQueueLock)
	std::mutex taskQueueLock;
	std::set<EIcore_runner_object*, EIcore_runner_object_cmp> taskQueue;
	std::condition_variable taskCv; // notify-one once task queue gets updated 

	// Thread-management (locked via threadMgmtLock)
	std::mutex threadMgmtLock;
	volatile bool doRun = true;
	int64_t threadCountRunning = 0; // The count of the threads, which are currently effectively running - without guest-threads
	std::condition_variable threadSuspensionCv; // notify-one once another thread will be freed 
	size_t threadCountSuspended = 0; // The count of threads that are currently waiting to be reactivated
	int64_t consecutiveFedCycles = 0; // Consecutive cycles without task shortage
	std::list<EIcore_runner_thread> threads; // !!! Never edit if doRun=false
	volatile bool scheduleCleanThreads = false;

	struct EIcore_runner_dbg {
		enum RegisterReason {
			INTERNAL,
			RESURRECT,
			GUEST,
			RUNNER,
			RUNNER_CLOSE_SUSPENDED

		};

		static std::string regReasonName(RegisterReason reason);

		std::mutex registerLock;
		std::map<std::thread::id, std::pair<RegisterReason, bool>> registered;
		int64_t guestCount;
		std::mutex resLock;
		std::map<EIcore_runner_object*, std::thread::id> resMap;

	};

	EIcore_runner_dbg* dbg;

	void dbg_init();
	void dbg_cleanup();

	// Registration-Debug (thread safe, but shpuldn't have to be) 
	// TODO Panic if used "un-threasafe"
	void dbg_giveRes(EIcore_runner_object* obj);
	void dbg_recieveRes(EIcore_runner_object* obj);
	void dbg_registerRunning(std::thread::id tid, EIcore_runner_dbg::RegisterReason reason=EIcore_runner_dbg::INTERNAL);
	void dbg_registerRunning(EIcore_runner_dbg::RegisterReason reason);
	void dbg_unregisterRunning(EIcore_runner_dbg::RegisterReason reason);

	void registerRunning();
	void unregisterRunning();

	void cleanThreads();
	void spawnThread(bool collapse);
	
	// Interface creation-counter (not thread-safe)
	int64_t interfaceIdCounter = 0;

	// Internal functions (not thread-safe)
	void stop();

	// Internal functions (thread-safe if concurrentInterface=true)
	void run(EIcore_runner_thread* threadHandle, bool collapse);
	int32_t enqueue(EIcore_runner_object* obj);

	// Thread-management-tools (thread-safe / autolocking threadMgmtLock)

	/**
	 * @brief  Request a new thread
	 * @param  mode: Szenario in which the thread is requested
	 * @retval If a thread could be registered
	 */
	bool requestNewThread(EIcore_runner_THREAD_REQUEST_MODE mode=NEW);
public:
	/**
	 * @brief  Creates single-threaded runner, which will do things just-in-time on fetch
	 * @param  concurrent: Enable/Disable thread-safeness
	 */
	explicit EIcore_runner(bool useQueue = true, bool concurrent = true);
	/**
	 * @brief  Creates multi-threaded runner
	 * @param  maxRunning: The maximum number of actively running threads
	 */
	explicit EIcore_runner(size_t maxRunning);
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
	EIcore_runner_object*const parent;
	EIcore_runner*const runner;
	const int64_t renderId;
	const std::vector<int64_t>*const prioStack;


	// Sub-task-data (locked by "subTasksLock")
	std::mutex subTasksLock;
	int64_t renderIdCounter = 0;
	std::vector<EIcore_runner_object*>* subTasks = NULL;
	uint32_t subTaskSize = 0;

	// Own results (locked by "resultLock")
	std::mutex resultLock;
	RenderResult*volatile result = nullptr;
	std::condition_variable* resultCv = nullptr;
	std::chrono::_V2::system_clock::time_point* resultCreation = nullptr;

	// Progress status (locked by statusLock)
	std::mutex statusLock; // Locks status
	volatile EIcore_runner_object_status status = PENDING; // Status of task
	std::condition_variable unsuspendCv;

	void suspend();
	void unsuspend();

	std::vector<EIcore_runner_object*>* getSubTaskMemory(size_t maxIndex);

protected:
	EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, const std::vector<int64_t>*);
	EIcore_runner_object(EIcore_runner* runner, int64_t renderId, const std::vector<int64_t>*);
	virtual ~EIcore_runner_object();

	RenderResult* run(std::function<void()>* outCleanupFunction);
	RenderResult* fetchOwnRenderResult();

	void setRenderContext(RenderContext* rctx);
	void setResultSettings(ResultSettings* rs);

public:
	uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio) override;
	void fetchRenderResults(ResultPair* requests, size_t requestCount) override;
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
