#include "../include/torasu/std/Rstring_replace.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>

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

Rstring_replace::Rstring_replace(Renderable* src, Renderable* before, Renderable* after)
	: SimpleRenderable("EXAMPLE::RSTRING_REPLACE", false, true),
	  srcRnd(src), beforeRnd(before), afterRnd(after) {}


Rstring_replace::~Rstring_replace() {}

torasu::ResultSegment* Rstring_replace::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {

		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();

		// Sub-renderings

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		auto renderIdSrc = rib.enqueueRender(srcRnd, rctx, ei);
		auto renderIdBefore = rib.enqueueRender(beforeRnd, rctx, ei);
		auto renderIdAfter = rib.enqueueRender(afterRnd, rctx, ei);

		std::unique_ptr<torasu::RenderResult> rrSrc(ei->fetchRenderResult(renderIdSrc));
		std::unique_ptr<torasu::RenderResult> rrBefore(ei->fetchRenderResult(renderIdBefore));
		std::unique_ptr<torasu::RenderResult> rrAfter(ei->fetchRenderResult(renderIdAfter));

		auto fetchedSrc = segHandle.getFrom(rrSrc.get());
		auto fetchedBefore = segHandle.getFrom(rrBefore.get());
		auto fetchedAfter = segHandle.getFrom(rrAfter.get());

		// Evaluation

		if (fetchedSrc.getResult() == nullptr) {
			throw std::runtime_error("Source for replacement cant be provided!");
		}

		std::string srcStr = fetchedSrc.getResult()->getString();

		if (fetchedBefore.getResult() == nullptr || fetchedAfter.getResult() == nullptr) {
			// TODO note that one of the results couldnt be provided
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK_WARN, new torasu::tstd::Dstring(srcStr), true);
		}

		std::string beforeStr = fetchedBefore.getResult()->getString();
		std::string afterStr = fetchedAfter.getResult()->getString();

		if (beforeStr.length() < 1) {
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dstring(srcStr), true);
		}

		findAndReplaceAll(&srcStr, beforeStr, afterStr);

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dstring(srcStr), true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_replace::getElements() {
	torasu::ElementMap elems;
	elems["ex"] = srcRnd;
	return elems;
}

void Rstring_replace::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("ex", &srcRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd