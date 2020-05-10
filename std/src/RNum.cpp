#include "../include/torasu/std/RNum.hpp"

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

#include <torasu/std/DPNum.hpp>

using namespace std;

namespace torasu::tstd {

RNum::RNum(double val) : SimpleRenderable("STD::RNUM", true) {
	valdr = new DPNum(val);
}

RNum::~RNum() {
	delete valdr;
}

DataResource* RNum::getData() {
	return valdr;
}

void RNum::setData(DataResource* data) {
	if (DPNum* dpnum = dynamic_cast<DPNum*>(data)) {
		delete valdr;
		valdr = dpnum;
	} else {
		throw invalid_argument("The data-type \"DPNum\" is only allowed");
	}
}

ResultSegment* RNum::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline().compare(pipeline) == 0) {
		return new ResultSegment(ResultSegmentStatus_OK, valdr, false);
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}

}

} // namespace torasu::tstd
