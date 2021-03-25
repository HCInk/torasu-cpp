#ifndef CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_

#include <string>
#include <set>
#include <map>
#include <memory>
#include <vector>
#include <sstream>
#include <stdexcept>

#include <torasu/torasu.hpp>
#include <torasu/RenderableProperties.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/log_tools.hpp>

namespace torasu::tools {

//
// RenderTools
//

template<class T> class CastedRenderSegmentResult;

/**
 * @brief  Tool to help making operations inside a renderable easier
 * 		- Manages: Logging and building of info ref; Result-masking and -generation
 */
class RenderHelper {
public:
	ExecutionInterface* const ei;
	LogInstruction li;
	RenderContext* const rctx;
private:
	RenderContextMask* resMask;
public:
	torasu::tools::LogInfoRefBuilder lrib;

	explicit RenderHelper(RenderInstruction* ri);
	RenderHelper(ExecutionInterface* ei, LogInstruction li, RenderContext* rctx);
	~RenderHelper();

	/** @brief  Will collect another rctx-mask into the result-mask
	 * - The generated mask will be used in buildResult(..) or can be manually fetched with takeResMask() */
	void collectMask(const RenderContextMask* mask);

	/** @brief  Takes the result-mask based on the masks collected via collectMask(...)
	 * @note   Should only be called once - Second calls will give an invalid result - Is also called in buildResult(...) */
	RenderContextMask* takeResMask();

	/** @brief  Builds a result without a payload (uses internal tools to enrich the result with the requested data)
	 * @param  status: The status of the result
	 * @retval The genrated segment (managed by caller) */
	ResultSegment* buildResult(ResultSegmentStatus status);

	/** @brief  Builds a result with a payload (uses internal tools to enrich the result with the requested data)
	 * @param  dr: The payload of the result
	 * @param  status: The status of the result (if OK and warnings/erros have been collected in lrib, will be set to OK_WARN)
	 * @retval The genrated segment (managed by caller) */
	ResultSegment* buildResult(DataResource* dr, ResultSegmentStatus status = ResultSegmentStatus_OK);

	/** @brief Checks if logs with the given log-level may be logged */
	inline bool mayLog(torasu::LogLevel level) const {
		return li.level <= level;
	}

	//
	// Passthru / Aliases
	//

	/** @brief Fetch render result from ExecutionIntrface */
	inline torasu::RenderResult* fetchRenderResult(uint64_t rid) {
		return ei->fetchRenderResult(rid);
	}
};


template<class T> class CastedRenderSegmentResult {
private:
	T* result;
	ResultSegmentStatus status;
	ResultSegment* rs;
	torasu::LogId infoTag = LogId_MAX;
	torasu::tools::LogInfoRefBuilder* infoBuilder = nullptr;
public:

	explicit CastedRenderSegmentResult(ResultSegmentStatus status, LogId infoTag = LogId_MAX, tools::LogInfoRefBuilder* infoBuilder = nullptr)
		: result(nullptr), status(status), rs(nullptr), infoTag(infoTag), infoBuilder(infoBuilder)  {}

	explicit CastedRenderSegmentResult(ResultSegment* rs, tools::LogInfoRefBuilder* infoBuilder = nullptr)
		: status(rs->getStatus()), rs(rs), infoBuilder(infoBuilder)  {
		DataResource* result = rs->getResult();
		if (result == nullptr) {
			this->result = nullptr;
		} else if (T* casted = dynamic_cast<T*>(result)) {
			this->result = casted;
		} else {
			this->result = nullptr;
			std::ostringstream errMsg;
			errMsg << "Returned object is not of the expected type\""
				   << typeid(T).name()
				   << "\"!";
			if (infoBuilder != nullptr) {
				infoBuilder->hasError = true;
				infoTag = infoBuilder->logCause(WARN, errMsg.str(), new auto(*getRawInfo()));
				return;
			} else {
				throw std::logic_error(errMsg.str());
			}
		}

		if (status == ResultSegmentStatus_OK) return;

		if (infoBuilder != nullptr) {
			infoBuilder->hasError = true;
			switch (status) {
			case ResultSegmentStatus_OK_WARN:
				infoTag = infoBuilder->logCause(WARN, "Sub-render is marked to contain errors", new auto(*getRawInfo()));
				break;
			default:
				infoTag = infoBuilder->logCause(WARN, "Sub-render returned with abnormal status " + std::to_string(status),
												new auto(*getRawInfo()));
				break;
			}
		}
	}

	~CastedRenderSegmentResult() {}

	inline T* getResult() const {
		return result;
	}

	inline const RenderContextMask* getResultMask() const {
		return rs != nullptr ? rs->getResultMask() : nullptr;
	}

	inline bool canFreeResult() const {
		return rs ? rs->canFreeResult() : false;
	}

	inline T* ejectResult() {
		return rs ? dynamic_cast<T*>(rs->ejectResult()) : nullptr;
	}

	inline ResultSegmentStatus getStatus() const {
		return status;
	}

	inline LogInfoRef* getRawInfo() const {
		return rs ? rs->getResultInfoRef() : nullptr;
	}

	inline LogId takeInfoTag() {
		if (infoBuilder != nullptr) infoBuilder->releaseCause(infoTag);
		return infoTag;
	}

	inline explicit operator bool() const noexcept {
		return result != nullptr;
	}
};

template<class T> inline CastedRenderSegmentResult<T> findResult(RenderResult* rr, const std::string& key, torasu::tools::LogInfoRefBuilder* infoBuilder = nullptr) {
	std::map<std::string, ResultSegment*>* results = rr->getResults();
	if (results == NULL) {
		if (infoBuilder != nullptr) {
			infoBuilder->hasError = true;
			auto causeTag = infoBuilder->logCause(WARN, "Generated results are empty!");
			return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT, causeTag, infoBuilder);
		} else {
			return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT);
		}
	}

	std::map<std::string, ResultSegment*>::iterator found = results->find(key);
	if (found != rr->getResults()->end()) {
		return CastedRenderSegmentResult<T>(found->second, infoBuilder);
	} else {
		if (infoBuilder != nullptr) {
			infoBuilder->hasError = true;
			auto causeTag = infoBuilder->logCause(WARN, "Generated result not found under key \"" + key + "\"!");
			return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT, causeTag, infoBuilder);
		} else {
			return CastedRenderSegmentResult<T>(ResultSegmentStatus::ResultSegmentStatus_NON_EXISTENT);
		}
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

	inline CastedRenderSegmentResult<T> getFrom(RenderResult* rr, torasu::tools::LogInfoRefBuilder* infoBuilder = nullptr) {
		return findResult<T>(rr, segKey, infoBuilder);
	}

	inline CastedRenderSegmentResult<T> getFrom(RenderResult* rr, torasu::tools::RenderHelper* helper, bool collectMask = true) {
		auto result = findResult<T>(rr, segKey, &helper->lrib);
		if (collectMask) helper->collectMask(result.getResultMask());
		return result;
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

	inline uint64_t enqueueRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, LogInstruction li, int64_t prio=0) {
#ifdef TORASU_CHECK_RENDER_NULL_RCTX
		if (rctx == nullptr) throw std::logic_error("Can't enqueue render without a render-context");
#endif
		return ei->enqueueRender(rnd, rctx, getResultSetttings(), li, prio);
	}

	inline uint64_t enqueueRender(RenderableSlot rnd, RenderContext* rctx, ExecutionInterface* ei, LogInstruction li, int64_t prio=0) {
		return enqueueRender(rnd.get(), rctx, ei, li, prio);
	}

	inline uint64_t enqueueRender(Renderable* rnd, RenderHelper* rh, RenderContext* rctx = nullptr, int64_t prio=0) {
		if (rctx == nullptr) rctx = rh->rctx;
		return rh->ei->enqueueRender(rnd, rctx, getResultSetttings(), rh->li, prio);
	}

	inline uint64_t enqueueRender(RenderableSlot rnd, RenderHelper* rh, RenderContext* rctx = nullptr, int64_t prio=0) {
		return enqueueRender(rnd.get(), rh, rctx, prio);
	}

	inline RenderResult* runRender(Renderable* rnd, RenderContext* rctx, ExecutionInterface* ei, LogInstruction li, int64_t prio=0) {
		uint64_t renderId = enqueueRender(rnd, rctx, ei, li, prio);
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

inline RenderableProperties* getProperties(Renderable* rnd, std::set<std::string> rProps, torasu::ExecutionInterface* ei, LogInstruction li, RenderContext* rctx = nullptr) {
	auto* rp = new RenderableProperties();

	std::unique_ptr<RenderContext> dummyRctx;
	if (rctx == nullptr) {
		dummyRctx = std::unique_ptr<RenderContext>(new RenderContext());
		rctx = dummyRctx.get();
	}

	ResultSettings rs;
	int32_t segmentKey = 0;
	for (std::string propKey : rProps) {
		ResultSegmentSettings* segSettings = new ResultSegmentSettings(TORASU_PROPERTY_PREFIX + propKey, std::to_string(segmentKey), nullptr);
		rs.push_back(segSettings);
		segmentKey++;
	}

	uint64_t rendId = ei->enqueueRender(rnd, rctx, &rs, li, 0);
	std::unique_ptr<RenderResult> result(ei->fetchRenderResult(rendId));

	for (ResultSegmentSettings* segSettings : rs) {
		delete segSettings;
	}

	if (result->getResults() == nullptr) {
		throw std::runtime_error("Error when rendering properties: Renderable failed to return result-map!");
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

	return rp;
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
