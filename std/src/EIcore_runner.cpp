#include "../include/torasu/std/EIcore_runner.hpp"

#include <sstream>
#include <memory>
// #include <iostream>

using namespace std;

#define MAX_RETRIES 50
#define RETRY_WAIT 10

namespace torasu::tstd {

//
// EICoreRunner
//

EIcore_runner::EIcore_runner() {
	spawnThread(false);
}

EIcore_runner::~EIcore_runner() {
	stop();
}

void EIcore_runner::stop() {
	threadMgmtLock.lock();
	doRun = false;
	threadMgmtLock.unlock();
	for (EIcore_runner_thread& thread : threads) {
		thread.thread->join();
		delete thread.thread;
	}
	threads.clear();
}

void EIcore_runner::spawnThread(bool collapse) {
	threads.push_back({});
	EIcore_runner_thread& threadHandle = threads.back();
	threadHandle.thread = new std::thread([this, &threadHandle, collapse]() {
		this->run(threadHandle, collapse);
	});
	threadCountCurrent++;
}

void EIcore_runner::cleanThreads() {
	bool found;
	do {
		found = false;
		for (auto it = threads.begin(); it != threads.end(); it++) {
			auto& thread = *it;
			if (!thread.running) {
				thread.thread->join();
				delete thread.thread;
				threads.erase(it);
				// Re-search for thread to clean
				found = true;
				break;
			}
		}
	} while(found);
}

void EIcore_runner::run(EIcore_runner_thread& threadHandle, bool collapse) {
	
	size_t retriesWithNone = 0;
	bool suspended = false;

	while (doRun) {

		//
		// Suspension-Management
		//

		while (suspended && doRun) {
			
			if (requestNewThread()) {
				suspended = false;
				break;
			}

			if ((!collapse || retriesWithNone < MAX_RETRIES)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT));
				continue;
			} else {
				break;
			}
			
		}

		//
		// Get task
		//

		taskQueueLock.lock();

		EIcore_runner_object* task = nullptr;

		for (auto* currTask : taskQueue) {
			auto& statusLock = currTask->statusLock;

			if (currTask->status == PENDING) {
				statusLock.lock();
				if (currTask->status == PENDING) { 
					currTask->status = RUNNING;
					statusLock.unlock();
					task = currTask;
					break;
				} else {
					statusLock.unlock();
				}
			} else if (currTask->status == SUSPENDED) {
				statusLock.lock();
				if (currTask->status == SUSPENDED) { 
					// Unsuspend / Transfer run-privilege to target
					currTask->status = RUNNING;
					statusLock.unlock();

					// Try to request a new run-privilege for current thread, if not suspend
					suspended = !requestNewThread();

					if (suspended) {
						break; 	
					}
				} else {
					statusLock.unlock();
				}
			}
		}
		
		taskQueueLock.unlock();

		if (task == nullptr) { // Wait for task if no task is available / has been suspended
			if ((!collapse || retriesWithNone < MAX_RETRIES)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT));
				continue;
			} else {
				break;
			}
		}

		//
		// Running the task
		//

		std::function<void(void)> runCleanup;
		RenderResult* result = task->run(&runCleanup);
		task->resultLock.lock();
		task->result = result;
		task->resultLock.unlock();

		runCleanup();

		//
		// Removing task from queue
		//

		taskQueueLock.lock();
		auto found = taskQueue.find(task);
		taskQueue.erase(found);
		taskQueueLock.unlock();

		//
		// Thread-cleanup routine
		//

		if (scheduleCleanThreads) {
			threadMgmtLock.lock();
			if (!doRun) { // Cancel if doRun=false - since threads may not be edited in that state
				threadMgmtLock.unlock();
				break;
			}
			if (scheduleCleanThreads) {
				scheduleCleanThreads = false;
				cleanThreads();
			}
			threadMgmtLock.unlock();
		}
	}

	threadMgmtLock.lock();
	threadHandle.running = false;
	if (!suspended) threadCountCurrent--;
	scheduleCleanThreads = true;
	threadMgmtLock.unlock();
}

int32_t EIcore_runner::enqueue(EIcore_runner_object* obj) {
	taskQueueLock.lock();
	taskQueue.insert(obj);
	
	// std::cout << " ==== TASK QUEUE ====" << std::endl;
	// for (auto cObj : taskQueue) {
	// 	std::cout << "- ";
	// 	for (auto prio : *cObj->prioStack) {
	// 		std::cout << " " << prio;
	// 	}
	// 	std::cout << std::endl;
	// }

	taskQueueLock.unlock();



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
EIcore_runner_object::EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, std::vector<int64_t>* prioStack) {
	
	this->elemHandler = EIcore_runner_elemhandler::getHandler(rnd, runner);

	this->rnd = rnd;

	this->parent = parent;
	this->runner = runner;
	this->renderId = renderId;

	this->prioStack = prioStack;

}

// Interface Constructor
EIcore_runner_object::EIcore_runner_object(EIcore_runner* runner, int64_t renderId, std::vector<int64_t>* prioStack) {
	this->elemHandler = nullptr;
	this->rnd = nullptr;
	this->parent = nullptr;

	this->runner = runner;
	this->renderId = renderId;

	this->prioStack = prioStack;
	this->status = RUNNING;

}

EIcore_runner_object::~EIcore_runner_object() {
	if (subTasks != NULL) {
		delete subTasks;
	}

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
	// Singlethreaded just-in-time for now
	// std::function<void()> cleanup;
	// result = run(&cleanup); 
	// cleanup();

	// 0 = Not set to sleep yet, 1=Set to sleep, 2=cant be set to sleep
	int suspendState = 0;

	while (true) {
		resultLock.lock();
		if (result != NULL) {
			if (suspendState == 1) parent->unsuspend();
			return result;
		}
		resultLock.unlock();
		if (suspendState == 0) {
			if (parent->parent != nullptr) { // Check if parent is not an interface
				parent->suspend();
				suspendState = 1;
			} else {
				suspendState = 2;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

//
// Execution Interface
//

uint64_t EIcore_runner_object::enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio) {
	subTasksLock.lock();
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

	subTasksLock.unlock();

	runner->enqueue(obj);

	return newRenderId;
}

RenderResult* EIcore_runner_object::fetchRenderResult(uint64_t renderId) {
	subTasksLock.lock();
	if (subTasks != NULL && /*renderId >= 0 &&*/ ((uint32_t)renderId) < subTaskSize) {

		auto it = subTasks->begin()+renderId;
		EIcore_runner_object* obj = *it;
		*it = NULL;
		subTasksLock.unlock();


		if (obj != NULL) {

			RenderResult* rr = obj->fetchOwnRenderResult();
			// delete obj; // XXX for debugging
			return rr;

		} else {
			subTasksLock.unlock();
			std::ostringstream errMsg;
			errMsg << "The object the given render-id ("
				   << renderId
				   << "), is reffering to, has already been fetched and freed!";
			throw runtime_error(errMsg.str());
		}

	} else {
		subTasksLock.unlock();
		std::ostringstream errMsg;
		errMsg << "The given render-id ("
			   << renderId
			   << ") was never created!";
		throw runtime_error(errMsg.str());
	}

}

void EIcore_runner_object::lock(LockId lockId) {
	suspend();
	elemHandler->lock(lockId);
	unsuspend();
}

void EIcore_runner_object::unlock(LockId lockId) {
	elemHandler->unlock(lockId);
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
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
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