#include "../include/torasu/std/Rnumber_string.hpp"

#include <memory>
#include <cmath>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>

namespace torasu::tstd {

Rnumber_string::Rnumber_string(torasu::tstd::NumSlot src, torasu::tstd::NumSlot decimals, torasu::tstd::NumSlot minDigits)
	: SimpleRenderable(false, true), srcRnd(src), decimalsRnd(decimals), minDigitsRnd(minDigits) {}

Rnumber_string::~Rnumber_string() {}

Identifier Rnumber_string::getType() {
	return "STD::RNUM_STR";
}

torasu::RenderResult* Rnumber_string::render(torasu::RenderInstruction* ri) {
	auto pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {
		tools::RenderHelper rh(ri);

		torasu::ResultSettings numSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
		auto ridSrc = rh.enqueueRender(srcRnd, &numSetting);
		auto ridDec = rh.enqueueRender(decimalsRnd, &numSetting);
		auto ridDig = rh.enqueueRender(minDigitsRnd, &numSetting);
		std::unique_ptr<RenderResult> rrSrc(rh.fetchRenderResult(ridSrc));
		std::unique_ptr<RenderResult> rrDec(rh.fetchRenderResult(ridDec));
		std::unique_ptr<RenderResult> rrDig(rh.fetchRenderResult(ridDig));

		auto src = rh.evalResult<tstd::Dnum>(rrSrc.get());
		if (!src) {
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide source number for number-string.", src.takeInfoTag());

			return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
		}
		double number = src.getResult()->getNum();

		auto decimalPlacesRes = rh.evalResult<tstd::Dnum>(rrDec.get());
		int64_t decimalPlaces;
		if (!decimalPlacesRes) {
			decimalPlaces = 0;
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide decimal-places, displaying no places", decimalPlacesRes.takeInfoTag());
		} else {
			decimalPlaces = std::lround(decimalPlacesRes.getResult()->getNum());
		}

		auto digitMinRes = rh.evalResult<tstd::Dnum>(rrDig.get());
		int64_t digitMin;
		if (!digitMinRes) {
			digitMin = 1;
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide minimum digit-count, setting no minimum", digitMinRes.takeInfoTag());
		} else {
			digitMin = std::lround(digitMinRes.getResult()->getNum());
			if (digitMin < 1) digitMin = 1;
		}

		std::string numberStr = std::to_string(std::lround(number*(std::pow(10, decimalPlaces))));

		int64_t splitPoint = numberStr.size()-decimalPlaces;

		if (splitPoint < digitMin) {
			size_t padSize = digitMin-splitPoint;
			std::string padStr;
			for (size_t i = 0; i < padSize; i++) {
				padStr += "0";
			}
			numberStr = padStr + numberStr;
			splitPoint += padSize;
		}

		std::string formatted = (decimalPlaces > 0) ?
								(numberStr.substr(0, splitPoint) + "." + numberStr.substr(splitPoint) )
								: numberStr;

		return rh.buildResult(new tstd::Dstring(formatted));

	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rnumber_string::getElements() {
	torasu::ElementMap elemMap;

	elemMap["src"] = srcRnd.get();
	elemMap["dec"] = decimalsRnd.get();
	elemMap["dig"] = minDigitsRnd.get();

	return elemMap;
}

void Rnumber_string::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("src", &srcRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("dec", &decimalsRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("dig", &minDigitsRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd