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

	bool makeReady = rdyObjs.get()->size() > 0;

	if (makeReady) {
		ReadyInstruction rdyInstr(*rdyObjs.get(), this);
		rnd->ready(rdyInstr);
	}

	RenderInstruction ri(rctx, rs, this);

	RenderResult* res = rnd->render(&ri);

	resultLock.lock();
	result = res;
	resultLock.unlock();
	
	if (makeReady) {
		UnreadyInstruction urdyInstr(*rdyObjs.get());
		rnd->unready(urdyInstr);
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

} // namespace torasu::tstd