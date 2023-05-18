#include "../include/torasu/std/Radd.hpp"

#include <string>
#include <optional>
#include <chrono>

#include <torasu/render_tools.hpp>

#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace torasu::tstd {

Radd::Radd(NumSlot a, NumSlot b)
	: SimpleRenderable(false, true), a(a), b(b) {}

Radd::~Radd() {

}

Identifier Radd::getType() {
	return "STD::RADD";
}

RenderResult* Radd::render(RenderInstruction* ri) {

	tools::RenderHelper rh(ri);
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		std::unique_ptr<RenderResult> resA(rh.fetchRenderResult(rendA));
		std::unique_ptr<RenderResult> resB(rh.fetchRenderResult(rendB));

		auto a = rh.evalResult<Dnum>(resA.get());
		auto b = rh.evalResult<Dnum>(resB.get());

		if (a && b) {
			double calcResult = a.getResult()->getNum() + b.getResult()->getNum();
			return rh.buildResult(new Dnum(calcResult));
		} else {
			if (rh.mayLog(WARN)) {
				torasu::tools::LogInfoRefBuilder errorCauses(rh.lrib.linstr);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				rh.lrib.logCause(WARN, "Sub render failed to provide operands, returning 0", errorCauses);
			}

			return rh.buildResult(new Dnum(0), RenderResultStatus_OK_WARN);
		}

	} else if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_VIS) {
		Dbimg_FORMAT* fmt;
		if (!(fmt = rh.getFormat<Dbimg_FORMAT>())) {
			return new RenderResult(RenderResultStatus_INVALID_FORMAT);
		}

		torasu::tools::ResultSettingsSingleFmt resSetting(TORASU_STD_PL_VIS, fmt);

		// Sub-Renderings

		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		RenderResult* resA = rh.fetchRenderResult(rendA);
		RenderResult* resB = rh.fetchRenderResult(rendB);

		// Calculating Result from Results

		auto a = rh.evalResult<Dbimg>(resA);
		auto b = rh.evalResult<Dbimg>(resB);

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

			auto li = ri->getLogInstruction();

			bool doBench = li.level <= LogLevel::DEBUG;
			std::chrono::time_point<std::chrono::steady_clock> bench;
			if (doBench) bench = std::chrono::steady_clock::now();

			int16_t buf;
			uint8_t currentPremulFactor;
			for (int i = dataSize-1; i >= 0; ) {
				// ALPHA
				dest[i] = srcA[i];
				currentPremulFactor = srcB[i];
				i--;
				// BLUE
				buf = (int16_t) srcA[i] + ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf <= 0xFF ? buf:0;
				i--;
				// GREEN
				buf = (int16_t) srcA[i] + ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf <= 0xFF ? buf:0;
				i--;
				// RED
				buf = (int16_t) srcA[i] + ( ((uint16_t) srcB[i]*currentPremulFactor) >>8);
				dest[i] = buf <= 0xFF ? buf:0;
				i--;
			}

			if (doBench) li.logger->log(LogLevel::DEBUG,
											"Add Time = " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - bench).count()) + "[ms]");

		}

		delete resA;
		delete resB;

		if (result != nullptr) {
			return rh.buildResult(result);
		} else {
			return rh.buildResult(new Dbimg(*fmt), RenderResultStatus_OK_WARN);
		}

	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT);
	}

}

ElementMap Radd::getElements() {
	ElementMap elems;

	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

const torasu::OptElementSlot Radd::setElement(std::string key, const ElementSlot* elem) {
	if (key == "a") return NumSlot::trySetRenderableSlot(&a, elem, 0);
	if (key == "b") return NumSlot::trySetRenderableSlot(&b, elem, 0);
	return nullptr;
}

} // namespace torasu::tstd