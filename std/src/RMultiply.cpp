#include "../include/torasu/std/RMultiply.hpp"

#include <string>
#include <map>
#include <optional>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/tools.hpp>
#include <torasu/std/DPNum.hpp>

using namespace std;

namespace torasu::tstd {

RMultiply::RMultiply(Renderable* a, Renderable* b) : resHandle(rib.addSegmentWithHandle<DPNum>(pipeline, NULL)) {
	this->a = a;
	this->b = b;
}

RMultiply::~RMultiply() {}

string RMultiply::getType() {
	return ident;
}

DataResource* RMultiply::getData() {
	return NULL;
}

map<string, Element*> RMultiply::getElements() {
	map<string, Element*> elems;
	
	elems["a"] = a;
	elems["b"] = b;

	return elems;
}

void RMultiply::setData(DataResource* data, map<string, Element*> elements) {
	// TODO Handle setData in RMultiply
}

void RMultiply::setData(DataResource* data) {
	// TODO Handle setData in RMultiply
}

void RMultiply::setElement(string key, Element* elem) {
	// TODO Handle setElement in RMultiply
}

RenderResult* RMultiply::render(RenderInstruction* ri) {
	ResultSettings* rs = ri->getResultSettings();

	for (ResultSegmentSettings* rss : *rs) {
		if (rss->getPipeline().compare(pipeline) == 0) {

			// Sub-Renderings

			auto ei = ri->getExecutionInterface();
			auto rctx = ri->getRenderContext();

			auto rendA = rib.enqueueRender(a, rctx, ei);
			auto rendB = rib.enqueueRender(b, rctx, ei);

			RenderResult* resA = ei->fetchRenderResult(rendA);
			RenderResult* resB = ei->fetchRenderResult(rendB);

			// Calculating Result from Results

			optional<double> calcResult;

			if (resA->getResults()!=NULL && resB->getResults()!=NULL) {
				tools::CastedRenderSegmentResult<DPNum>* a = resHandle.getFrom(resA);
				tools::CastedRenderSegmentResult<DPNum>* b = resHandle.getFrom(resA);
				
				if (a!=NULL && b!=NULL && a->getResult()!=NULL && b->getResult()!=NULL) {
					calcResult = a->getResult()->getNum() * b->getResult()->getNum();
					delete a;
					delete b;
				} else {

					if (a != NULL) {
						delete a;
					}
					if (b != NULL) {
						delete b;
					}
				}

			}

			// Free sub-results

			delete resA;
			delete resB;

			// Saving Result

			ResultSegment* rseg;
			if (calcResult.has_value()) {
				DPNum* mulRes = new DPNum(calcResult.value());
				rseg = new ResultSegment(ResultSegmentStatus::ResultSegmentStatus_OK, mulRes, true);
			} else {
				DPNum* errRes = new DPNum(0);
				rseg = new ResultSegment(ResultSegmentStatus::ResultSegmentStatus_OK_WARN, errRes, true);
			}

			map<string, ResultSegment*>* results = new map<string, ResultSegment*>();
			(*results)[rss->getKey()] = rseg;

			return new RenderResult(ResultStatus::ResultStatus_OK, results);

		}
	}

	return new RenderResult(ResultStatus::ResultStatus_MALFORMED);
}

} // namespace torasu::tstd