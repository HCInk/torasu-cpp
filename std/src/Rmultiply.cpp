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

	torasu::tools::RenderHelper rh(ri);
	auto& lirb = rh.lrib;

	const auto selPipleine = resSettings->getPipeline();

	if (selPipleine == TORASU_STD_PL_NUM) {

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dnum> resHandle = rib.addSegmentWithHandle<Dnum>(TORASU_STD_PL_NUM, NULL);

		auto rendA = rib.enqueueRender(a, &rh);
		auto rendB = rib.enqueueRender(b, &rh);

		std::unique_ptr<RenderResult> resA(rh.fetchRenderResult(rendA));
		std::unique_ptr<RenderResult> resB(rh.fetchRenderResult(rendB));

		tools::CastedRenderSegmentResult<Dnum> a = resHandle.getFrom(resA.get(), &rh);
		tools::CastedRenderSegmentResult<Dnum> b = resHandle.getFrom(resB.get(), &rh);

		if (a && b) {
			Dnum* mulRes = new Dnum(a.getResult()->getNum() * b.getResult()->getNum());
			return rh.buildResult(mulRes);
		} else {
			if (rh.mayLog(WARN)) {
				torasu::tools::LogInfoRefBuilder errorCauses(lirb.linstr);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				lirb.logCause(WARN, "Sub render failed to provide operands, returning 0", errorCauses);
			}

			return rh.buildResult(new Dnum(0), ResultSegmentStatus_OK_WARN);
		}

	} else if (selPipleine == TORASU_STD_PL_VIS) {
		Dbimg_FORMAT* fmt;
		auto fmtSettings = resSettings->getResultFormatSettings();
		if ( !( fmtSettings != nullptr
				&& (fmt = dynamic_cast<Dbimg_FORMAT*>(fmtSettings)) )) {
			return new ResultSegment(ResultSegmentStatus_INVALID_FORMAT);
		}

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dbimg> resHandle = rib.addSegmentWithHandle<Dbimg>(TORASU_STD_PL_VIS, fmt);

		// Sub-Renderings

		auto rendA = rib.enqueueRender(a, &rh);
		auto rendB = rib.enqueueRender(b, &rh);

		std::unique_ptr<RenderResult> resA(rh.fetchRenderResult(rendA));
		std::unique_ptr<RenderResult> resB(rh.fetchRenderResult(rendB));

		// Calculating Result from Results

		tools::CastedRenderSegmentResult<Dbimg> a = resHandle.getFrom(resA.get(), &rh);
		tools::CastedRenderSegmentResult<Dbimg> b = resHandle.getFrom(resB.get(), &rh);


		if (a && b) {
			Dbimg* result;
			if (a.canFreeResult()) result = a.ejectResult();
			else if (b.canFreeResult()) result = b.ejectResult();
			else result = new Dbimg(*fmt);

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


			return rh.buildResult(result);
		} else {

			if (rh.mayLog(WARN)) {
				torasu::tools::LogInfoRefBuilder errorCauses(lirb.linstr);
				if (!a)
					errorCauses.logCause(WARN, "Operand A failed to render", a.takeInfoTag());
				if (!b)
					errorCauses.logCause(WARN, "Operand B failed to render", b.takeInfoTag());

				lirb.logCause(WARN, "Sub render failed to provide operands, returning empty image", errorCauses);

			}

			Dbimg* errRes = new Dbimg(*fmt);
			errRes->clear();
			return rh.buildResult(errRes, ResultSegmentStatus_OK_WARN);
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