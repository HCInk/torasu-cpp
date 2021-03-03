#include "../include/torasu/std/Rmultiply.hpp"

#include <memory>
#include <string>
#include <optional>
#include <chrono>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/log_tools.hpp>

#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace torasu::tstd {

Rmultiply::Rmultiply(NumSlot a, NumSlot b)
	: SimpleRenderable(std::string("STD::RMULTIPLY"), false, true), a(a), b(b) {}

Rmultiply::~Rmultiply() {

}

ResultSegment* Rmultiply::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (numPipeline.compare(resSettings->getPipeline()) == 0) {

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dnum> resHandle = rib.addSegmentWithHandle<Dnum>(numPipeline, NULL);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto li = ri->getLogInstruction();
		auto rctx = ri->getRenderContext();
		torasu::tools::LogInfoRefBuilder lirb(li);

		auto rendA = rib.enqueueRender(a, rctx, ei, li);
		auto rendB = rib.enqueueRender(b, rctx, ei, li);

		std::unique_ptr<RenderResult> resA(ei->fetchRenderResult(rendA));
		std::unique_ptr<RenderResult> resB(ei->fetchRenderResult(rendB));

		tools::CastedRenderSegmentResult<Dnum> a = resHandle.getFrom(resA.get(), &lirb);
		tools::CastedRenderSegmentResult<Dnum> b = resHandle.getFrom(resB.get(), &lirb);

		if (a && b) {
			Dnum* mulRes = new Dnum(a.getResult()->getNum() * b.getResult()->getNum());
			return new ResultSegment(lirb.hasError ? ResultSegmentStatus_OK_WARN : ResultSegmentStatus_OK,
									 mulRes, true, lirb.build());
		} else {
			if (li.level <= WARN) {
				torasu::tools::LogInfoRefBuilder errorCauses(li);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				lirb.logCause(WARN, "Sub render failed to provide operands, returning 0", errorCauses);

			}

			Dnum* errRes = new Dnum(0);
			return new ResultSegment(ResultSegmentStatus_OK_WARN, errRes, true, lirb.build());
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

			for (size_t i = 0; i < dataSize; i++) {
				// dest[i] = (srcA[i]>>4)*(srcB[i]>>4);
				dest[i] = ((uint16_t) srcA[i]*srcB[i]) >> 8;
				// *dest++ = ((uint16_t) *srcA++ * *srcB++) >> 8;
			}

			if (doBench) li.logger->log(LogLevel::DEBUG,
											"Mul Time = " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - bench).count()) + "[ms]");

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

map<string, Element*> Rmultiply::getElements() {
	map<string, Element*> elems;

	elems["a"] = a.get();
	elems["b"] = b.get();

	return elems;
}

void Rmultiply::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("a", &a, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("b", &b, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd