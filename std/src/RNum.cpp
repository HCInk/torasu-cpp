#include "../include/torasu/std/RNum.hpp"

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/std/DPNum.hpp>

using namespace torasu;
using namespace std;

namespace torasu::tstd {

RNum::RNum(double val) {
	valdr = new DPNum(val);
}

RNum::~RNum() {
	delete valdr;
}

string RNum::getType() {
	return ident;
}

DataResource* RNum::getData() {
	return valdr;
}

map<string, Element*> RNum::getElements() {
	return map<string, Element*>();
}

void RNum::setData(DataResource* data, map<string, Element*> elements) {
	// TODO Handle setData in RNum
}

void RNum::setData(DataResource* data) {
	// TODO Handle setData in RNum
}

void RNum::setElement(string key, Element* elem) {
	// TODO Handle setElement in RNum
}

RenderResult* RNum::render(RenderInstruction* ri) {
	ResultSettings* rs = ri->getResultSettings();

	for (ResultSegmentSettings* rss : *rs) {
		if (rss->getPipeline().compare(pipeline) == 0) {
			ResultSegment* rseg = new ResultSegment(ResultSegmentStatus::ResultSegmentStatus_OK, valdr, false);

			map<string, ResultSegment*>* results = new map<string, ResultSegment*>();

			(*results)[rss->getKey()] = rseg;

			return new RenderResult(ResultStatus::ResultStatus_OK, results);
		}
	}

	return new RenderResult(ResultStatus::ResultStatus_MALFORMED);
}

} // namespace torasustd
