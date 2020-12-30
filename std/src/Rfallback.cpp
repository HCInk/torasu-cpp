#include "../include/torasu/std/Rfallback.hpp"

#include <memory>

namespace torasu::tstd {

Rfallback::Rfallback(std::vector<torasu::tools::RenderableSlot> slots)
	: SimpleRenderable("STD::RFALLBACK", false, true) {

	int i = 0;
	for (auto& slot : slots) {
		this->slots[i] = slot;
		i++;
	}

}

Rfallback::~Rfallback() {}

ResultSegment* Rfallback::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	auto* ei = ri->getExecutionInterface();

	// If no option succeeded, this level will indicate a possible cause of the error
	// The higher the level the more important it is
	enum {
		// 0: None of the renderables provided a segment, everything as expected
		NO_SEGMENTS = 0,
		// 1: Invalid Format one has given an format-error
		INVALID_FORMAT = 1,
		// 2: An error occurred when processing one result
		INTERNAL_ERROR = 2,
	} fallbackLevel = NO_SEGMENTS;

	for (auto& slot : slots) {
		torasu::ResultSettings rs = {resSettings};
		auto rid = ei->enqueueRender(slot.second.get(), ri->getRenderContext(), &rs, ri->getLogInstruction(), 0);

		std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rid));

		auto* res = rr->getResults();

		auto foundResult = res->find(resSettings->getKey());

		if (foundResult == res->end()) {
			// Error: This element has not provided the requested segment
			continue;
		}

		auto* rseg = foundResult->second;
		ResultSegmentStatus status = rseg->getStatus();

		if (status < 0) {
			switch (status) {
			case ResultSegmentStatus_INVALID_SEGMENT:
				// Signalizes that no resource is a available:
				// Planned use of fallback
				break;
			case ResultSegmentStatus_INVALID_FORMAT:
				// Error: Note invalid format
				if (fallbackLevel < INVALID_FORMAT) fallbackLevel = INVALID_FORMAT;
				break;
			case ResultSegmentStatus_INTERNAL_ERROR:
				// Error: Note Internal error
				if (fallbackLevel < INTERNAL_ERROR) fallbackLevel = INTERNAL_ERROR;
				break;
			default:
				// Error: Note unexpected status-code
				if (fallbackLevel < INTERNAL_ERROR) fallbackLevel = INTERNAL_ERROR;
				break;
			}
			continue;
		}

		torasu::ResultSegmentStatus resStatus =
			fallbackLevel == NO_SEGMENTS ? ResultSegmentStatus_OK : ResultSegmentStatus_OK_WARN;

		if (rseg->canFreeResult()) {
			return new ResultSegment(resStatus, rseg->ejectResult(), true);
		} else {
			return new ResultSegment(resStatus, rseg->getResult(), false);
		}


	}

	// Return code, if none was found

	torasu::ResultSegmentStatus resStatus;

	switch (fallbackLevel) {
	case NO_SEGMENTS:
		resStatus = ResultSegmentStatus_INVALID_SEGMENT;
		break;
	case INVALID_FORMAT:
		resStatus = ResultSegmentStatus_INVALID_FORMAT;
		break;
	case INTERNAL_ERROR:
		resStatus = ResultSegmentStatus_INTERNAL_ERROR;
		break;
	default:
		// Shouldn't happen
		resStatus = ResultSegmentStatus_INTERNAL_ERROR;
		break;
	}

	return new ResultSegment(resStatus);

}

torasu::ElementMap Rfallback::getElements() {
	torasu::ElementMap elems;
	for (auto& slot : slots) {
		elems[std::to_string(slot.first)] = slot.second.get();
	}
	return elems;
}

void Rfallback::setElement(std::string key, torasu::Element* elem) {
	size_t id;
	try {
		id = std::stoul(key);
	} catch (const std::exception& ex) {
		throw torasu::tools::makeExceptSlotDoesntExist(key);
	}

	if (elem == nullptr) {
		slots[id] = nullptr;
		return;
	}

	if (auto* rnd = dynamic_cast<Renderable*>(elem)) {
		slots[id] = rnd;
	} else {
		throw torasu::tools::makeExceptSlotOnlyRenderables(key);
	}
}

} // namespace torasu::tstd
