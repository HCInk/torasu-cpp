#ifndef CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

#include <torasu/torasu.hpp>
#include <torasu/RenderableProperties.hpp>
#include <torasu/slot_tools.hpp>

namespace torasu::tools {

//
// RenderTools
//

template<class T> class CastedRenderSegmentResult {
private:
	T* result;
	ResultSegmentStatus status;
	ResultSegment* rs;
public:

	explicit CastedRenderSegmentResult(ResultSegmentStatus status)  {
		this->status = status;
		this->result = nullptr;
		this->rs = nullptr;
	}

	explicit CastedRenderSegmentResult(ResultSegment* rs)  {
		this->rs = rs;
		this->status = rs->getStatus();
		DataResource* result = rs->getResult();
		if (result == nullptr) {
			this->result = nullptr;
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

	inline bool canFreeResult() {
		return rs ? rs->canFreeResult() : false;
	}

	inline T* ejectResult() {
		return rs ? dynamic_cast<T*>(rs->ejectResult()) : nullptr;
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

	inline uint64_t enqueueRender(RenderableSlot rnd, RenderContext* rctx, ExecutionInterface* ei, int64_t prio=0) {
		enqueueRender(rnd.get(), rctx, ei, prio);
	}

	inline uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, int64_t prio=0) {
#ifdef TORASU_CHECK_RENDER_NULL_RCTX
		if (rctx == nullptr) throw std::logic_error("Can't enqueue render without a render-context");
#endif
		return ei->enqueueRender(rnd, rctx, getResultSetttings(), prio);
	}

	inline RenderResult* runRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, int64_t prio=0) {
		uint64_t renderId = enqueueRender(rnd, rctx, ei, prio);
		return ei->fetchRenderResult(renderId);
	}
};

template<class T> inline T* getPropertyValue(RenderableProperties* props, std::string key, bool* incorrectType=nullptr) {
	auto& holder = (*props)[key];
	if (holder.get() == nullptr) {
		return nullptr;
	}
	if (T* casted = dynamic_cast<T*>(holder.get())) {
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
	auto* rp = new RenderableProperties();

	bool dummyRctx = rctx == nullptr;
	if (dummyRctx) {
		rctx = new RenderContext();
	}

	ResultSettings rs;
	int32_t segmentKey = 0;
	for (std::string propKey : rProps) {
		ResultSegmentSettings* segSettings = new ResultSegmentSettings(TORASU_PROPERTY_PREFIX + propKey, std::to_string(segmentKey), nullptr);
		rs.push_back(segSettings);
		segmentKey++;
	}

	uint64_t rendId = ei->enqueueRender(rnd, rctx, &rs, 0);
	RenderResult* result = ei->fetchRenderResult(rendId);

	for (ResultSegmentSettings* segSettings : rs) {
		delete segSettings;
	}

	segmentKey = 0;
	for (std::string propKey : rProps) {
		auto* segResult = (*result->getResults())[std::to_string(segmentKey)];
		if (segResult->getResult() != nullptr) {
			auto& dr = (*rp)[propKey];
			if (segResult->canFreeResult()) {
				dr.initialize(segResult->ejectResult(), true);
			} else {
				dr.initialize(segResult->getResult(), false);
			}
		}
		segmentKey++;
	}

	delete result;

	if (dummyRctx) {
		delete rctx;
	}
	return rp;
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
