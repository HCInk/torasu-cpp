#ifndef CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

#include <torasu/torasu.hpp>

namespace torasu::tools {

//
// RenderTools
//

template<class T> class CastedRenderSegmentResult {
private:
	T* result;
	ResultSegmentStatus status;
public:

	explicit CastedRenderSegmentResult(ResultSegmentStatus status)  {
		this->status = status;
		this->result = NULL;
	}

	explicit CastedRenderSegmentResult(ResultSegment* rs)  {
		this->status = rs->getStatus();
		DataResource* result = rs->getResult();
		if (result == NULL) {
			this->result = NULL;
		} else if (T* casted = dynamic_cast<T*>(result)) {
			this->result = casted;
		} else {
			std::ostringstream errMsg;
			errMsg << "Returned object is not of the expected type\""
				   << typeid(T).name()
				   << "\"!";
			throw std::logic_error(errMsg.str());
		}
	}

	~CastedRenderSegmentResult() {}

	inline T* getResult() {
		return result;
	}

	inline ResultSegmentStatus getStatus() {
		return status;
	}
};

template<class T> inline CastedRenderSegmentResult<T> findResult(RenderResult* rr, const std::string& key) {
	std::map<std::string, ResultSegment*>* results = rr->getResults();
	if (results == NULL) {
		return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT);
	}
	std::map<std::string, ResultSegment*>::iterator found = results->find(key);
	if (found != rr->getResults()->end()) {
		return CastedRenderSegmentResult<T>(found->second);
	} else {
		return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT);
	}

}

template<class T> class RenderResultSegmentHandle {
private:
	std::string segKey;
public:
	explicit RenderResultSegmentHandle(std::string segKey) {
		this->segKey = segKey;
	}

	~RenderResultSegmentHandle() {}

	inline CastedRenderSegmentResult<T> getFrom(RenderResult* rr) {
		return findResult<T>(rr, segKey);
	}

};

class RenderInstructionBuilder {
private:
	ResultSettings* rs = NULL;
	std::vector<ResultSegmentSettings*>* segments;

	void buildResultSettings();

	inline ResultSettings* getResultSetttings() {
		if (rs != NULL) {
			return rs;
		} else {
			buildResultSettings();
			return rs;
		}
	}

	int autoKeyIndex = 0;

	inline std::string getUnusedKey() {
		autoKeyIndex++;
		return std::to_string(autoKeyIndex-1);
	}

public:
	RenderInstructionBuilder();
	~RenderInstructionBuilder();

	inline void addSegment(const std::string& pipeline, const std::string& key, ResultFormatSettings* format) {
		segments->push_back(new ResultSegmentSettings(pipeline, key, format));
	}

	template<class T> inline RenderResultSegmentHandle<T> addSegmentWithHandle(const std::string& pipeline, ResultFormatSettings* format) {
		std::string segKey = getUnusedKey();
		addSegment(pipeline, segKey, format);
		return RenderResultSegmentHandle<T>(segKey);
	}

	inline uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, int64_t prio=0) {
		return ei->enqueueRender(rnd, rctx, getResultSetttings(), prio);
	}

	inline RenderResult* runRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, int64_t prio=0) {
		uint64_t renderId = enqueueRender(rnd, rctx, ei, prio);
		return ei->fetchRenderResult(renderId);
	}
};

template<class T> inline T* getPropertyValue(RenderableProperties* props, std::string key, bool* incorrectType=nullptr) {
	torasu::DataResource* value = (*props)[key];
	if (value == nullptr) {
		return nullptr;
	}
	if (T* casted = dynamic_cast<T*>(value)) {
		return casted;
	}
	if (incorrectType != nullptr) {
		*incorrectType = true;
	} else {
		throw std::runtime_error(std::string("Property-value in \"") + 
				key + std::string("\" was expected to be ") + typeid(T).name() + 
				std::string(", but couldn't be converted to the given type."));
	}
	
}

inline RenderableProperties* getProperties(Renderable* rnd, std::set<std::string> rProps, torasu::ExecutionInterface* ei, RenderContext* rctx = nullptr) {
	bool dummyRctx = rctx == nullptr;
	if (dummyRctx) {
		rctx = new RenderContext();
	}
	auto* pi = new PropertyInstruction(rProps, rctx, ei);
	auto* rp = rnd->getProperties(pi);
	delete pi;
	if (dummyRctx) {
		delete rctx;
	}
	return rp;
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
