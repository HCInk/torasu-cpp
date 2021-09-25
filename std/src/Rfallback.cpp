#include "../include/torasu/std/Rfallback.hpp"

#include <memory>
#include <torasu/render_tools.hpp>

namespace torasu::tstd {

Rfallback::Rfallback(std::vector<torasu::tools::RenderableSlot> slots)
	: SimpleRenderable(false, true) {

	int i = 0;
	for (auto& slot : slots) {
		this->slots[i] = slot;
		i++;
	}

}

Rfallback::~Rfallback() {}

Identifier Rfallback::getType() {
	return "STD::RFALLBACK";
}

RenderResult* Rfallback::render(RenderInstruction* ri) {

	tools::RenderHelper rh(ri);

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

	bool logWarns = rh.mayLog(torasu::WARN);
	bool logDetails = rh.mayLog(torasu::DEBUG);

	size_t optNo = 0;
	for (auto& slot : slots) {
		optNo++;
		auto rid = rh.ei->enqueueRender(slot.second.get(), rh.rctx, ri->getResultSettings(), rh.li, 0);

		std::unique_ptr<RenderResult> rr(rh.fetchRenderResult(rid));
		rh.collectMask(rr->getResultMask());

		RenderResultStatus status = rr->getStatus();

		if (status < 0) {
			switch (status) {
			case RenderResultStatus_INVALID_SEGMENT:
				// Signalizes that no resource is a available:
				// Planned use of fallback
				if (logDetails) rh.lrib.logCause(torasu::DEBUG, "Option No. " + std::to_string(optNo)
													 + " signalized that no result is present. Skipping. (As planned on no result-presence)",
													 new auto(*rr->getResultInfoRef()));
				break;
			case RenderResultStatus_INVALID_FORMAT:
				// Error: Note invalid format
				rh.lrib.hasError = true;
				if (logWarns) rh.lrib.logCause(torasu::WARN, "Option No. " + std::to_string(optNo)
												   + " encountered a format error, skipping to next fallback.",
												   new auto(*rr->getResultInfoRef()));
				if (fallbackLevel < INVALID_FORMAT) fallbackLevel = INVALID_FORMAT;
				break;
			case RenderResultStatus_INTERNAL_ERROR:
				// Error: Note Internal error
				rh.lrib.hasError = true;
				if (logWarns) rh.lrib.logCause(torasu::WARN, "Option No. " + std::to_string(optNo)
												   + " encountered an internal error, skipping to next fallback.",
												   new auto(*rr->getResultInfoRef()));
				if (fallbackLevel < INTERNAL_ERROR) fallbackLevel = INTERNAL_ERROR;
				break;
			default:
				// Error: Note unexpected status-code
				rh.lrib.hasError = true;
				if (logWarns) rh.lrib.logCause(torasu::WARN, "Option No. " + std::to_string(optNo)
												   + " encountered an unknown error-code (" + std::to_string(status) + "), skipping to next fallback.",
												   new auto(*rr->getResultInfoRef()));
				if (fallbackLevel < INTERNAL_ERROR) fallbackLevel = INTERNAL_ERROR;
				break;
			}
			continue;
		} else if (status == RenderResultStatus_OK_WARN) {
			rh.lrib.hasError = true;
			if (logWarns) rh.lrib.logCause(torasu::WARN, "Option No. " + std::to_string(optNo)
											   + " signalized that it may contain errors. Still taking.",
											   new auto(*rr->getResultInfoRef()));
		}

		if (logDetails) rh.li.logger->log(torasu::DEBUG, "Selected Option No. " + std::to_string(optNo));

		if (fallbackLevel != NO_SEGMENTS) rh.lrib.hasError = true;

		if (rr->canFreeResult()) {
			return rh.buildResult(rr->ejectResult());
		} else {
			return new RenderResult(!rh.lrib.hasError ? torasu::RenderResultStatus_OK: torasu::RenderResultStatus_OK_WARN,
									rr->getResult(), false, rh.takeResMask(), rh.lrib.build());
		}


	}

	// Return code, if none was found

	torasu::RenderResultStatus resStatus;

	switch (fallbackLevel) {
	case NO_SEGMENTS:
		resStatus = RenderResultStatus_INVALID_SEGMENT;
		break;
	case INVALID_FORMAT:
		resStatus = RenderResultStatus_INVALID_FORMAT;
		break;
	case INTERNAL_ERROR:
		resStatus = RenderResultStatus_INTERNAL_ERROR;
		break;
	default:
		// Shouldn't happen
		resStatus = RenderResultStatus_INTERNAL_ERROR;
		break;
	}

	if (logWarns) rh.lrib.logCauseSummary(torasu::WARN, "No matching fallback applied."
											  " (" + std::to_string(optNo) + " options checked)");

	return rh.buildResult(resStatus);

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
