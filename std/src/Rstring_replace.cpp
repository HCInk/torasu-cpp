#include "../include/torasu/std/Rstring_replace.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace {

inline void findAndReplaceAll(std::string* data, const std::string& toSearch, const std::string& replaceStr) {
	// Get the first occurrence
	size_t pos = data->find(toSearch);
	// Repeat till end is reached
	while( pos != std::string::npos) {
		// Replace this occurrence of Sub String
		data->replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data->find(toSearch, pos + replaceStr.size());
	}
}

} // namespace


namespace torasu::tstd {

Rstring_replace::Rstring_replace(StringSlot src, StringSlot before, StringSlot after)
	: SimpleRenderable("STD::RSTRING_REPLACE", false, true),
	  srcRnd(src), beforeRnd(before), afterRnd(after) {}


Rstring_replace::~Rstring_replace() {}

torasu::ResultSegment* Rstring_replace::render(torasu::RenderInstruction* ri) {
	std::string pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {

		tools::RenderHelper rh(ri);

		// Sub-renderings

		torasu::ResultSettings resultSettings(TORASU_STD_PL_STRING, nullptr);

		auto renderIdSrc = rh.enqueueRender(srcRnd, &resultSettings);
		auto renderIdBefore = rh.enqueueRender(beforeRnd, &resultSettings);
		auto renderIdAfter = rh.enqueueRender(afterRnd, &resultSettings);

		// XXX Maybe find a better way then this for multi-fetches
		torasu::ExecutionInterface::ResultPair results[] = {
			{renderIdSrc},
			{renderIdBefore},
			{renderIdAfter}
		};
		rh.fetchRenderResults(results, sizeof(results)/sizeof(torasu::ExecutionInterface::ResultPair));
		std::unique_ptr<torasu::ResultSegment> rrSrc(results[0].result);
		std::unique_ptr<torasu::ResultSegment> rrBefore(results[1].result);
		std::unique_ptr<torasu::ResultSegment> rrAfter(results[2].result);

		auto fetchedSrc = rh.evalResult<torasu::tstd::Dstring>(rrSrc.get());
		auto fetchedBefore = rh.evalResult<torasu::tstd::Dstring>(rrBefore.get());
		auto fetchedAfter = rh.evalResult<torasu::tstd::Dstring>(rrAfter.get());

		// Evaluation

		if (fetchedSrc.getResult() == nullptr) {
			if (rh.mayLog(torasu::WARN)) {
				rh.lrib.logCause(torasu::WARN, "Source for replacement cant be provided!", fetchedSrc.takeInfoTag());
			}
			rh.buildResult(torasu::ResultSegmentStatus_INTERNAL_ERROR);
		}

		std::string srcStr = fetchedSrc.getResult()->getString();

		if (!fetchedBefore || !fetchedAfter) {
			// TODO note that one of the results couldnt be provided
			if (rh.mayLog(torasu::WARN)) {

				torasu::tools::LogInfoRefBuilder errorCauses(rh.lrib.linstr);
				if (!fetchedBefore)
					errorCauses.logCause(WARN, "Before-string failed to render.", fetchedBefore.takeInfoTag());
				if (!fetchedAfter)
					errorCauses.logCause(WARN, "After-string failed to render.", fetchedAfter.takeInfoTag());

				rh.lrib.logCause(WARN, "Sub render failed to provide operands, returning 0", errorCauses);
			}

			return rh.buildResult(new torasu::tstd::Dstring(srcStr), torasu::ResultSegmentStatus_OK_WARN);
		}

		std::string beforeStr = fetchedBefore.getResult()->getString();
		std::string afterStr = fetchedAfter.getResult()->getString();

		if (!beforeStr.empty()) {
			findAndReplaceAll(&srcStr, beforeStr, afterStr);
		}

		return rh.buildResult(new torasu::tstd::Dstring(srcStr));
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_replace::getElements() {
	torasu::ElementMap elems;
	elems["src"] = srcRnd.get();
	elems["old"] = beforeRnd.get();
	elems["new"] = afterRnd.get();
	return elems;
}

void Rstring_replace::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("src", &srcRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("old", &beforeRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("new", &afterRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd