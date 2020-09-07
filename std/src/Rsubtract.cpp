#include "../include/torasu/std/Rsubtract.hpp"

#include <string>
#include <map>
#include <optional>
#include <chrono>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/Dnum.hpp>
#include <torasu/std/Dbimg.hpp>

using namespace std;

namespace torasu::tstd {

Rsubtract::Rsubtract(Renderable* a, Renderable* b)
	: SimpleRenderable(std::string("STD::RSUBTRACT"), false, true) {
	this->a = a;
	this->b = b;
}

Rsubtract::~Rsubtract() {

}

ResultSegment* Rsubtract::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (numPipeline.compare(resSettings->getPipeline()) == 0) {

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dnum> resHandle = rib.addSegmentWithHandle<Dnum>(numPipeline, NULL);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto rctx = ri->getRenderContext();

		auto rendA = rib.enqueueRender(a, rctx, ei);
		auto rendB = rib.enqueueRender(b, rctx, ei);

		RenderResult* resA = ei->fetchRenderResult(rendA);
		RenderResult* resB = ei->fetchRenderResult(rendB);

		// Calculating Result from Results

		std::optional<double> calcResult;

		tools::CastedRenderSegmentResult<Dnum> a = resHandle.getFrom(resA);
		tools::CastedRenderSegmentResult<Dnum> b = resHandle.getFrom(resB);

		if (a.getResult()!=NULL && b.getResult()!=NULL) {
			calcResult = a.getResult()->getNum() - b.getResult()->getNum();
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
				&& resSettings->getResultFormatSettings()->getFormat() == "STD::DBIMG"
				&& (fmt = dynamic_cast<Dbimg_FORMAT*>(resSettings->getResultFormatSettings()->getData())) )) {
			return new ResultSegment(ResultSegmentStatus_INVALID_FORMAT);
		}

		tools::RenderInstructionBuilder rib;
		auto fmtHandle = fmt->asFormat();
		tools::RenderResultSegmentHandle<Dbimg> resHandle = rib.addSegmentWithHandle<Dbimg>(visPipeline, &fmtHandle);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto rctx = ri->getRenderContext();

		auto rendA = rib.enqueueRender(a, rctx, ei);
		auto rendB = rib.enqueueRender(b, rctx, ei);

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

			auto benchBegin = std::chrono::steady_clock::now();

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

			auto benchEnd = std::chrono::steady_clock::now();
			std::cout << "  Sub Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(benchEnd - benchBegin).count() << "[ms] " << std::chrono::duration_cast<std::chrono::microseconds>(benchEnd - benchBegin).count() << "[us]" << std::endl;

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

map<string, Element*> Rsubtract::getElements() {
	map<string, Element*> elems;

	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

void Rsubtract::setElement(std::string key, Element* elem) {

	if (key.compare("a") == 0) {

		if (elem == NULL) {
			throw invalid_argument("Element slot \"a\" may not be empty!");
		}
		if (Renderable* rnd = dynamic_cast<Renderable*>(elem)) {
			a = rnd;
			return;
		} else {
			throw invalid_argument("Element slot \"a\" only accepts Renderables!");
		}

	} else if (key.compare("b") == 0) {

		if (elem == NULL) {
			throw invalid_argument("Element slot \"b\" may not be empty!");
		}
		if (Renderable* rnd = dynamic_cast<Renderable*>(elem)) {
			b = rnd;
			return;
		} else {
			throw invalid_argument("Element slot \"b\" only accepts Renderables!");
		}

	} else {
		std::ostringstream errMsg;
		errMsg << "The element slot \""
			   << key
			   << "\" does not exist!";
		throw invalid_argument(errMsg.str());
	}

}

} // namespace torasu::tstd