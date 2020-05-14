#ifndef CORE_INCLUDE_TORASU_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_TOOLS_HPP_

#include <string>
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

	explicit RenderResultSegmentHandle(const RenderResultSegmentHandle<T>& handle) {
		this->segKey = handle.segKey;
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


} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_TOOLS_HPP_
