#include "../include/torasu/std/EIcore_runner.hpp"

#include <sstream>
#include <memory>

using namespace std;

namespace torasu::tstd {

//
// EICoreRunner
//

EIcore_runner::EIcore_runner() {

}

EIcore_runner::~EIcore_runner() {

}

int32_t EIcore_runner::enqueue(EIcore_runner_object* obj) {
	return -1; // TODO Placeholder
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

	return new EIcore_runner_object(NULL, NULL, this, newInterfaceId, selectedPrioStack);
}

//
// EICoreRunnerObject
//

EIcore_runner_object::EIcore_runner_object(Renderable* rnd, EIcore_runner_object* parent, EIcore_runner* runner, int64_t renderId, std::vector<int64_t>* prioStack) {

	if (rnd != nullptr) {
		this->elemHandler = EIcore_runner_elemhandler::getHandler(rnd, runner);
	} else {
		this->elemHandler = nullptr;
	}

	this->rnd = rnd;

	this->parent = parent;
	this->runner = runner;
	this->renderId = renderId;

	this->prioStack = prioStack;

}

EIcore_runner_object::~EIcore_runner_object() {
	if (subTasks != NULL) {
		delete subTasks;
	}

	delete prioStack;
}

void EIcore_runner_object::run() {
	std::vector<std::string> ops;
	for (auto& rss : *rs) {
		ops.push_back(rss->getPipeline());
	}

	ReadyRequest rdyRequest(ops, rctx);
	std::unique_ptr<ReadyObjects> rdyObjs(rnd->requestReady(rdyRequest));

	bool makeReady = rdyObjs.get() != nullptr;

	if (makeReady) {
		elemHandler->readyElement(*rdyObjs.get(), this);
	}

	RenderInstruction ri(rctx, rs, this);

	RenderResult* res = rnd->render(&ri);

	resultLock.lock();
	result = res;
	resultLock.unlock();

	if (makeReady) {
		elemHandler->unreadyElement(*rdyObjs.get());
	}
}

RenderResult* EIcore_runner_object::fetchOwnRenderResult() {
	run(); // Singlethreaded just-in-time for now
	while (true) {
		resultLock.lock();
		if (result != NULL) {
			return result;
		}
		resultLock.unlock();
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
	size_t thisPrioStackSize = prioStack->size();
	std::vector<int64_t>* newPrioStack = new std::vector<int64_t>(thisPrioStackSize+2);
	(*newPrioStack)[thisPrioStackSize] = prio;
	(*newPrioStack)[thisPrioStackSize+1] = newRenderId;

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
			delete obj;
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
	elemHandler->lock(lockId, &blocked);
	waitSuspension();
}

void EIcore_runner_object::unlock(LockId lockId) {
	elemHandler->unlock(lockId);
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

void EIcore_runner_elemhandler::lock(LockId lockId, volatile bool* blocked) {

	lockStatesLock.lock();
	auto& lock = lockStates[lockId];
	lockStatesLock.unlock();

	*blocked = true;
	lock.lock();
	*blocked = false;

}

void EIcore_runner_elemhandler::unlock(LockId lockId) {
	lockStatesLock.lock();
	auto& lock = lockStates[lockId];
	lockStatesLock.unlock();

	lock.unlock();

}

} // namespace torasu::tstd