#ifndef STD_INCLUDE_TORASU_STD_EICORERUNNER_HPP_
#define STD_INCLUDE_TORASU_STD_EICORERUNNER_HPP_

#include <vector>
#include <map>
#include <mutex>

#include <torasu/torasu.hpp>


namespace torasu::tstd {

class EICoreRunnerObject;

class EICoreRunner {
protected:
	int32_t enqueue(EICoreRunnerObject* obj);
	int64_t interfaceIdCounter = 0;
public:
	EICoreRunner();
	~EICoreRunner();

	ExecutionInterface* createInterface(std::vector<int64_t>* prioStack=NULL);

	friend class EICoreRunnerObject;
};


class EICoreRunnerObject : public ExecutionInterface {
private:
	// Settings
	const size_t addAmmount = 10;

	// Object-data: Task-Information (persistent)
	Renderable* rnd;
	RenderContext* rctx = NULL;
	ResultSettings* rs = NULL;

	// Object-data: Task-Settings (persistent)
	int64_t renderId;
	EICoreRunnerObject* parent;
	EICoreRunner* runner;
	std::vector<int64_t>* prioStack;

	// Sub-task-data (locked by "subTasksLock")
	std::mutex subTasksLock;
	int64_t renderIdCounter = 0;
	std::vector<EICoreRunnerObject*>* subTasks = NULL;
	uint32_t subTaskSize = 0;

	// Own results (locked by "resultLock")
	std::mutex resultLock;
	RenderResult* result = NULL;

	inline std::vector<EICoreRunnerObject*>* getSubTaskMemory(size_t maxIndex) {
		if (subTasks == NULL) {
			subTasks = new std::vector<EICoreRunnerObject*>(addAmmount);
		}
		while (subTasks->size() <= maxIndex) {
			subTasks->resize(subTasks->size() + addAmmount);
		}
		return subTasks;
	}

protected:
	EICoreRunnerObject(Renderable* rnd, EICoreRunnerObject* parent, EICoreRunner* runner, int64_t renderId, std::vector<int64_t>* prioStack);
	virtual ~EICoreRunnerObject();

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

	friend class EICoreRunner;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_EICORERUNNER_HPP_
