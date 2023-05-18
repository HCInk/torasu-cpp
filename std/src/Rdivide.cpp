#include "../include/torasu/std/Rdivide.hpp"

#include <string>
#include <optional>
#include <chrono>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace torasu::tstd {

Rdivide::Rdivide(NumSlot a, NumSlot b)
	: SimpleRenderable(false, true), a(a), b(b) {}

Rdivide::~Rdivide() {}

Identifier Rdivide::getType() {
	return "STD::RDIVIDE";
}

RenderResult* Rdivide::render(RenderInstruction* ri) {

	tools::RenderHelper rh(ri);
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		RenderResult* resA = rh.fetchRenderResult(rendA);
		RenderResult* resB = rh.fetchRenderResult(rendB);

		// Calculating Result from Results

		std::optional<double> calcResult;

		auto a = rh.evalResult<Dnum>(resA);
		auto b = rh.evalResult<Dnum>(resB);

		if (a && b) {
			calcResult = a.getResult()->getNum() / b.getResult()->getNum();
		}

		// Free sub-results

		delete resA;
		delete resB;

		// Saving Result

		if (calcResult.has_value()) {
			Dnum* mulRes = new Dnum(calcResult.value());
			return new RenderResult(RenderResultStatus_OK, mulRes, true);
		} else {
			Dnum* errRes = new Dnum(0);
			return new RenderResult(RenderResultStatus_OK_WARN, errRes, true);
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
			for (size_t i = 0; i < dataSize; i++) {
				// dest[i] = (srcA[i]>>4)*(srcB[i]>>4);
				buf = ((int16_t) srcA[i] * 0xFF) / srcB[i];
				dest[i] = buf <= 0xFF ? buf : 0xFF;
				// *dest++ = ((uint16_t) *srcA++ * *srcB++) >> 8;
			}

			if (doBench) li.logger->log(LogLevel::DEBUG,
											"Div Time = " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - bench).count()) + "[ms]");

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

ElementMap Rdivide::getElements() {
	ElementMap elems;

	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

const torasu::OptElementSlot Rdivide::setElement(std::string key, const ElementSlot* elem) {
	if (key == "a") return NumSlot::trySetRenderableSlot(&a, elem, 1);
	if (key == "b") return NumSlot::trySetRenderableSlot(&b, elem, 1);
	return nullptr;
}

} // namespace torasu::tstd