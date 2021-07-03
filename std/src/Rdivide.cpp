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
	: SimpleRenderable(std::string("STD::RDIVIDE"), false, true),
	  a(a), b(b) {}

Rdivide::~Rdivide() {

}

ResultSegment* Rdivide::render(RenderInstruction* ri) {

	tools::RenderHelper rh(ri);
	if (numPipeline == ri->getResultSettings()->getPipeline()) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, nullptr);
		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		ResultSegment* resA = rh.fetchRenderResult(rendA);
		ResultSegment* resB = rh.fetchRenderResult(rendB);

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
			return new ResultSegment(ResultSegmentStatus_OK, mulRes, true);
		} else {
			Dnum* errRes = new Dnum(0);
			return new ResultSegment(ResultSegmentStatus_OK_WARN, errRes, true);
		}

	} else if (visPipeline == ri->getResultSettings()->getPipeline()) {
		Dbimg_FORMAT* fmt;
		if ( !( ri->getResultSettings()->getFromat() != nullptr
				&& (fmt = dynamic_cast<Dbimg_FORMAT*>(ri->getResultSettings()->getFromat())) )) {
			return new ResultSegment(ResultSegmentStatus_INVALID_FORMAT);
		}

		torasu::ResultSettings resSetting(TORASU_STD_PL_VIS, fmt);

		// Sub-Renderings

		auto rendA = rh.enqueueRender(a.get(), &resSetting);
		auto rendB = rh.enqueueRender(b.get(), &resSetting);

		ResultSegment* resA = rh.fetchRenderResult(rendA);
		ResultSegment* resB = rh.fetchRenderResult(rendB);

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
			std::chrono::_V2::steady_clock::time_point bench;
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
			return rh.buildResult(new Dbimg(*fmt), ResultSegmentStatus_OK_WARN);
		}

	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}

}

ElementMap Rdivide::getElements() {
	ElementMap elems;

	elems["a"] = a.get();
	elems["b"] = b.get();

	return elems;
}

void Rdivide::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("a", &a, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("b", &b, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd