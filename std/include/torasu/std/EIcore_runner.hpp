#ifndef STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_
#define STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_

#include <vector>
#include <map>
#include <mutex>

#include <torasu/torasu.hpp>

namespace torasu::tstd {

class EIcore_runner_object;

class EIcore_runner {
protected:
	int32_t enqueue(EIcore_runner_object* obj);
	int64_t interfaceIdCounter = 0;
public:
	EIcore_runner();
	~EIcore_runner();

	ExecutionInterface* createInterface(std::vector<int64_t>* prioStack=NULL);

	friend class EIcore_runner_object;
};


class EIcore_runner_object : public ExecutionInterface {
private:
	// Settings
	const size_t addAmmount = 10;

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
	RenderResult* result = NULL;

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
	virtual ~EIcore_runner_object();

	void run();
	RenderResult* fetchOwnRenderResult();

	inline void setRenderContext(RenderContext* rctx) {
		this->rctx = rctx;
	}

	inline void setResultSettings(ResultSettings* rs) {
		this->rs = rs;
	}

public:
	virtual uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ResultSettings* rs, int64_t prio);
	virtual RenderResult* fetchRenderResult(uint64_t renderId);

	friend class EIcore_runner;
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

class EIcore_runner_elemhandler {
private:
	Element* elem;
 	EIcore_runner* parent;
	std::mutex readyStatesLock; // Lock for readyStates and its contents
	std::map<ReadyObject, EIcore_runner_rdystate> readyStates;

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
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_EICORE_RUNNER_HPP_
