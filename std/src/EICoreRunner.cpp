#include "../include/torasu/std/EICoreRunner.hpp"

#include <sstream>

using namespace std;

namespace torasu::tstd {

//
// EICoreRunner
//

EICoreRunner::EICoreRunner() {

}

EICoreRunner::~EICoreRunner() {

}

int32_t EICoreRunner::enqueue(EICoreRunnerObject* obj) {
	return -1; // TODO Placeholder
}

ExecutionInterface* EICoreRunner::createInterface(std::vector<int64_t>* prioStack) {
	int64_t newInterfaceId = interfaceIdCounter;
	interfaceIdCounter++;

	std::vector<int64_t>* selectedPrioStack;
	if (prioStack != NULL) {
		selectedPrioStack = prioStack;
	} else {
		selectedPrioStack = new std::vector<int64_t>();
	}

	return new EICoreRunnerObject(NULL, NULL, this, newInterfaceId, selectedPrioStack);
}

//
// EICoreRunnerObject
//

EICoreRunnerObject::EICoreRunnerObject(Renderable* rnd, EICoreRunnerObject* parent, EICoreRunner* runner, int64_t renderId, std::vector<int64_t>* prioStack) {
	this->rnd = rnd;

	this->parent = parent;
	this->runner = runner;
	this->renderId = renderId;

	this->prioStack = prioStack;

}

EICoreRunnerObject::~EICoreRunnerObject() {
	if (subTasks != NULL) {
		delete subTasks;
	}

	delete prioStack;
}

void EICoreRunnerObject::run() {

	RenderInstruction ri(rctx, rs, this);

	RenderResult* res = rnd->render(&ri);

	resultLock.lock();
	result = res;
	resultLock.unlock();
}

RenderResult* EICoreRunnerObject::fetchOwnRenderResult() {
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

uint64_t EICoreRunnerObject::enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio) {
	subTasksLock.lock();
	// Select renderId
	uint64_t newRenderId = renderIdCounter;
	renderIdCounter++;

	// Make prio-stack: {x,x,x,x,prio,id}
	size_t thisPrioStackSize = prioStack->size();
	std::vector<int64_t>* newPrioStack = new std::vector<int64_t>(thisPrioStackSize+2);
	(*newPrioStack)[thisPrioStackSize] = prio;
	(*newPrioStack)[thisPrioStackSize+1] = newRenderId;

	EICoreRunnerObject* obj = new EICoreRunnerObject(rnd, this, runner, newRenderId, newPrioStack);

	obj->setRenderContext(rctx);
	obj->setResultSettings(rs);

	auto stm = getSubTaskMemory(newRenderId);
	(*stm)[newRenderId] = obj;
	subTaskSize = newRenderId+1;

	subTasksLock.unlock();

	runner->enqueue(obj);

	return renderId;
}

RenderResult* EICoreRunnerObject::fetchRenderResult(uint64_t renderId) {
	subTasksLock.lock();
	if (subTasks != NULL && /*renderId >= 0 &&*/ ((uint32_t)renderId) < subTaskSize) {

		auto it = subTasks->begin()+renderId;
		EICoreRunnerObject* obj = *it;
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

} // namespace torasu::tstd