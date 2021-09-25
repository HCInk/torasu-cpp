#include "../include/torasu/std/Rsubtract.hpp"

#include <string>
#include <optional>
#include <chrono>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace torasu::tstd {

Rsubtract::Rsubtract(NumSlot a, NumSlot b)
	: SimpleRenderable(false, true), a(a), b(b) {}

Rsubtract::~Rsubtract() {}

Identifier Rsubtract::getType() {
	return "STD::RSUBTRACT";
}

RenderResult* Rsubtract::render(RenderInstruction* ri) {
	torasu::tools::RenderHelper rh(ri);
	auto* resSettings = ri->getResultSettings();
	auto pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_NUM) {

		torasu::ResultSettings rs(TORASU_STD_PL_NUM, nullptr);
		auto rendA = rh.enqueueRender(a, &rs);
		auto rendB = rh.enqueueRender(b, &rs);

		RenderResult* resA = rh.fetchRenderResult(rendA);
		RenderResult* resB = rh.fetchRenderResult(rendB);

		// Calculating Result from Results

		std::optional<double> calcResult;

		tools::CastedRenderSegmentResult<tstd::Dnum> a = rh.evalResult<tstd::Dnum>(resA);
		tools::CastedRenderSegmentResult<tstd::Dnum> b = rh.evalResult<tstd::Dnum>(resB);

		if (a && b) {
			calcResult = a.getResult()->getNum() - b.getResult()->getNum();
		}

		// Free sub-results

		delete resA;
		delete resB;

		// Saving Result

		if (calcResult.has_value()) {
			Dnum* mulRes = new Dnum(calcResult.value());
			return rh.buildResult(mulRes);
		} else {
			Dnum* errRes = new Dnum(0);
			return rh.buildResult(errRes, RenderResultStatus_OK_WARN);
		}

	} else if (pipeline == TORASU_STD_PL_VIS) {
		Dbimg_FORMAT* fmt;
		if ( !( resSettings->getFromat() != NULL
				&& (fmt = dynamic_cast<Dbimg_FORMAT*>(resSettings->getFromat())) )) {
			return new RenderResult(RenderResultStatus_INVALID_FORMAT);
		}

		torasu::ResultSettings rs(TORASU_STD_PL_VIS, fmt);

		auto rendA = rh.enqueueRender(a, &rs);
		auto rendB = rh.enqueueRender(b, &rs);

		RenderResult* resA = rh.fetchRenderResult(rendA);
		RenderResult* resB = rh.fetchRenderResult(rendB);

		// Calculating Result from Results

		tools::CastedRenderSegmentResult<Dbimg> a = rh.evalResult<tstd::Dbimg>(resA);
		tools::CastedRenderSegmentResult<Dbimg> b = rh.evalResult<tstd::Dbimg>(resB);

		Dbimg* result = NULL;

		if (a && b) {

			result = new Dbimg(*fmt);

			const uint32_t height = result->getHeight();
			const uint32_t width = result->getWidth();
			const uint32_t channels = 4;
			const size_t dataSize = height*width*channels;


			uint8_t* srcA = a.getResult()->getImageData();
			uint8_t* srcB = b.getResult()->getImageData();
			uint8_t* dest = result->getImageData();

			bool doBench = rh.mayLog(torasu::DEBUG);
			std::chrono::_V2::steady_clock::time_point bench;
			if (doBench) bench = std::chrono::steady_clock::now();

			int16_t buf;
			uint8_t currentPremulFactor;
			for (int i = dataSize-1; i >= 0; ) {
				// ALPHA
				dest[i] = srcA[i];
				currentPremulFactor = srcB[i];
				i--;
				// BLUE
				buf = (int16_t) srcA[i] - ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf >= 0 ? buf:0;
				i--;
				// GREEN
				buf = (int16_t) srcA[i] - ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf >= 0 ? buf:0;
				i--;
				// RED
				buf = (int16_t) srcA[i] - ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf >= 0 ? buf:0;
				i--;
			}

			if (doBench) rh.li.logger->log(LogLevel::DEBUG,
											   "Sub Time = " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - bench).count()) + "[ms]");

		}

		delete resA;
		delete resB;

		if (result != nullptr) {
			return rh.buildResult(result);
		} else {
			Dbimg* errRes = new Dbimg(*fmt);
			return rh.buildResult(errRes, torasu::RenderResultStatus_OK_WARN);
		}

	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT);
	}

}

ElementMap Rsubtract::getElements() {
	ElementMap elems;

	elems["a"] = a.get();
	elems["b"] = b.get();

	return elems;
}

void Rsubtract::setElement(std::string key, Element* elem) {

	if (torasu::tools::trySetRenderableSlot("a", &a, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("b", &b, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);

}

} // namespace torasu::tstd