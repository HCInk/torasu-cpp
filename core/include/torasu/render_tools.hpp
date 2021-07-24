#ifndef CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_

#include <string>
#include <set>
#include <map>
#include <utility>
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
	explicit RenderHelper(ReadyInstruction* ri);
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

	inline uint64_t enqueueRender(torasu::Renderable* rend, torasu::ResultSettings* rs, torasu::RenderContext* rctx = nullptr, int64_t prio = 0) {
		return ei->enqueueRender(rend, rctx != nullptr ? rctx : this->rctx, rs, li, prio);
	}

	inline uint64_t enqueueRender(torasu::tools::RenderableSlot rend, torasu::ResultSettings* rs, torasu::RenderContext* rctx = nullptr, int64_t prio = 0) {
		return enqueueRender(rend.get(), rs, rctx, prio);
	}

	/** @brief Fetch render result from ExecutionIntrface */
	inline torasu::ResultSegment* fetchRenderResult(uint64_t rid) {
		return ei->fetchRenderResult(rid);
	}

	/** @brief Fetch render results from ExecutionIntrface */
	inline void fetchRenderResults(torasu::ExecutionInterface::ResultPair* requests, size_t requestCount) {
		ei->fetchRenderResults(requests, requestCount);
	}

	inline torasu::ResultSegment* runRender(torasu::Renderable* rend, torasu::ResultSettings* rs, torasu::RenderContext* rctx = nullptr, int64_t prio = 0) {
		return fetchRenderResult(enqueueRender(rend, rs, rctx, prio));
	}

	inline torasu::ResultSegment* runRender(torasu::tools::RenderableSlot rend, torasu::ResultSettings* rs, torasu::RenderContext* rctx = nullptr, int64_t prio = 0) {
		return runRender(rend.get(), rs, rctx, prio);
	}

	template<class T> inline CastedRenderSegmentResult<T> evalResult(torasu::ResultSegment* rr, bool doCollectMask = true) {
		auto result = CastedRenderSegmentResult<T>(rr, &lrib);
		if (doCollectMask) collectMask(result.getResultMask());
		return result;
	}

	template<class T> inline void noteSubError(CastedRenderSegmentResult<T> res, std::string message) {
		lrib.hasError = true;
		if (mayLog(torasu::WARN)) {
			lrib.logCause(torasu::WARN, message, res.takeInfoTag());
		}
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
				infoTag = infoBuilder->logCause(WARN, errMsg.str(), getRawInfoCopy() );
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
				infoTag = infoBuilder->logCause(WARN, "Sub-render is marked to contain errors", getRawInfoCopy() );
				break;
			default:
				infoTag = infoBuilder->logCause(WARN, "Sub-render returned with abnormal status " + std::to_string(status), getRawInfoCopy() );
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

	inline const LogInfoRef* getRawInfo() const {
		return rs ? rs->getResultInfoRef() : nullptr;
	}

	inline LogInfoRef* getRawInfoCopy() const {
		const auto* rawInfo = getRawInfo();
		return rawInfo != nullptr ? new auto(*rawInfo) : nullptr;
	}

	inline LogId takeInfoTag() {
		if (infoBuilder != nullptr) infoBuilder->releaseCause(infoTag);
		return infoTag;
	}

	inline explicit operator bool() const noexcept {
		return result != nullptr;
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

inline RenderableProperties* getProperties(Renderable* rnd, const std::set<std::string> rProps, torasu::ExecutionInterface* ei, LogInstruction li, RenderContext* rctx = nullptr) {
	auto* rp = new RenderableProperties();

	std::unique_ptr<RenderContext> dummyRctx;
	if (rctx == nullptr) {
		dummyRctx = std::unique_ptr<RenderContext>(new RenderContext());
		rctx = dummyRctx.get();
	}

	std::vector<std::pair<torasu::ExecutionInterface::ResultPair*, const char*>> resultMapping;
	std::vector<torasu::ResultSettings*> settingsStore;
	std::vector<torasu::ExecutionInterface::ResultPair> resPair(rProps.size());
	size_t i = 0;
	for (const auto& propKey : rProps) {
		ResultSettings* segSettings = new ResultSettings(propKey.c_str(), nullptr);
		settingsStore.push_back(segSettings);
		auto& rp = resPair[i];
		rp.renderId = ei->enqueueRender(rnd, rctx, segSettings, li, 0);
		resultMapping.push_back({&rp, propKey.c_str()});
		i++;
	}

	ei->fetchRenderResults(resPair.data(), i);

	for (ResultSettings* segSettings : settingsStore) {
		delete segSettings;
	}

	for (const auto& prop : resultMapping) {
		auto* segResult = prop.first->result;
		if (segResult->getResult() != nullptr) {
			auto& dr = (*rp)[prop.second];
			if (segResult->canFreeResult()) {
				dr.initialize(segResult->ejectResult(), true);
			} else {
				dr.initialize(segResult->getResult(), false);
			}
		}
	}

	return rp;
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_RENDER_TOOLS_HPP_
