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

ResultSegment* Rdivide::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (numPipeline.compare(resSettings->getPipeline()) == 0) {

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dnum> resHandle = rib.addSegmentWithHandle<Dnum>(numPipeline, NULL);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto li = ri->getLogInstruction();
		auto rctx = ri->getRenderContext();

		auto rendA = rib.enqueueRender(a, rctx, ei, li);
		auto rendB = rib.enqueueRender(b, rctx, ei, li);

		RenderResult* resA = ei->fetchRenderResult(rendA);
		RenderResult* resB = ei->fetchRenderResult(rendB);

		// Calculating Result from Results

		std::optional<double> calcResult;

		tools::CastedRenderSegmentResult<Dnum> a = resHandle.getFrom(resA);
		tools::CastedRenderSegmentResult<Dnum> b = resHandle.getFrom(resB);

		if (a.getResult()!=NULL && b.getResult()!=NULL) {
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

	} else if (visPipeline.compare(resSettings->getPipeline()) == 0) {
		Dbimg_FORMAT* fmt;
		if ( !( resSettings->getResultFormatSettings() != NULL
				&& (fmt = dynamic_cast<Dbimg_FORMAT*>(resSettings->getResultFormatSettings())) )) {
			return new ResultSegment(ResultSegmentStatus_INVALID_FORMAT);
		}

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dbimg> resHandle = rib.addSegmentWithHandle<Dbimg>(visPipeline, fmt);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto li = ri->getLogInstruction();
		auto rctx = ri->getRenderContext();

		auto rendA = rib.enqueueRender(a, rctx, ei, li);
		auto rendB = rib.enqueueRender(b, rctx, ei, li);

		RenderResult* resA = ei->fetchRenderResult(rendA);
		RenderResult* resB = ei->fetchRenderResult(rendB);

		// Calculating Result from Results

		tools::CastedRenderSegmentResult<Dbimg> a = resHandle.getFrom(resA);
		tools::CastedRenderSegmentResult<Dbimg> b = resHandle.getFrom(resB);

		Dbimg* result = NULL;

		if (a.getResult()!=NULL && b.getResult()!=NULL) {

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

		if (result != NULL) {
			return new ResultSegment(ResultSegmentStatus_OK, result, true);
		} else {
			Dbimg* errRes = new Dbimg(*fmt);
			return new ResultSegment(ResultSegmentStatus_OK_WARN, errRes, true);
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