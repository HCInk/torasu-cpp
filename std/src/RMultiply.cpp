#include "../include/torasu/std/RMultiply.hpp"

#include <string>
#include <map>
#include <optional>

#include <torasu/torasu.hpp>
#include <torasu/tools.hpp>

#include <torasu/std/DPNum.hpp>

using namespace std;

namespace torasu::tstd {

RMultiply::RMultiply(Renderable* a, Renderable* b)
	: SimpleRenderable(std::string("STD::RMULTIPLY")),
	  resHandle(rib.addSegmentWithHandle<DPNum>(pipeline, NULL)) {
	this->a = a;
	this->b = b;
}

RMultiply::~RMultiply() {

}

ResultSegment* RMultiply::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (pipeline.compare(resSettings->getPipeline())  == 0) {

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto rctx = ri->getRenderContext();

		auto rendA = rib.enqueueRender(a, rctx, ei);
		auto rendB = rib.enqueueRender(b, rctx, ei);

		RenderResult* resA = ei->fetchRenderResult(rendA);
		RenderResult* resB = ei->fetchRenderResult(rendB);

		// Calculating Result from Results

		std::optional<double> calcResult;

		tools::CastedRenderSegmentResult<DPNum> a = resHandle.getFrom(resA);
		tools::CastedRenderSegmentResult<DPNum> b = resHandle.getFrom(resB);

		if (a.getResult()!=NULL && b.getResult()!=NULL) {
			calcResult = a.getResult()->getNum() * b.getResult()->getNum();
		}

		// Free sub-results

		delete resA;
		delete resB;

		// Saving Result

		if (calcResult.has_value()) {
			DPNum* mulRes = new DPNum(calcResult.value());
			return new ResultSegment(ResultSegmentStatus_OK, mulRes, true);
		} else {
			DPNum* errRes = new DPNum(0);
			return new ResultSegment(ResultSegmentStatus_OK_WARN, errRes, true);
		}

	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}

}

map<string, Element*> RMultiply::getElements() {
	map<string, Element*> elems;

	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

void RMultiply::setElement(std::string key, Element* elem) {

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