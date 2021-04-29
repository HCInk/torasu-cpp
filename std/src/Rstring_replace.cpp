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

torasu::ResultSegment* Rstring_replace::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {

		tools::RenderHelper rh(ri);

		// Sub-renderings

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		auto renderIdSrc = rib.enqueueRender(srcRnd, &rh);
		auto renderIdBefore = rib.enqueueRender(beforeRnd, &rh);
		auto renderIdAfter = rib.enqueueRender(afterRnd, &rh);

		std::unique_ptr<torasu::RenderResult> rrSrc(rh.fetchRenderResult(renderIdSrc));
		std::unique_ptr<torasu::RenderResult> rrBefore(rh.fetchRenderResult(renderIdBefore));
		std::unique_ptr<torasu::RenderResult> rrAfter(rh.fetchRenderResult(renderIdAfter));

		auto fetchedSrc = segHandle.getFrom(rrSrc.get(), &rh);
		auto fetchedBefore = segHandle.getFrom(rrBefore.get(), &rh);
		auto fetchedAfter = segHandle.getFrom(rrAfter.get(), &rh);

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