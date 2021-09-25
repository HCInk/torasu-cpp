#include "../include/torasu/std/Rnum.hpp"

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

using namespace std;

namespace torasu::tstd {

Rnum::Rnum(double val) : SimpleRenderable(true, false) {
	valdr = new Dnum(val);
}

Rnum::Rnum(Dnum val) : SimpleRenderable(true, false) {
	valdr = new Dnum(val);
}

Rnum::~Rnum() {
	delete valdr;
}

Identifier Rnum::getType() {
	return "STD::RNUM";
}

DataResource* Rnum::getData() {
	return valdr;
}

void Rnum::setData(DataResource* data) {
	if (Dnum* dpnum = dynamic_cast<Dnum*>(data)) {
		delete valdr;
		valdr = dpnum;
	} else {
		throw invalid_argument("The data-type \"DNum\" is only allowed");
	}
}

ResultSegment* Rnum::render(RenderInstruction* ri) {

	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {
		return new ResultSegment(ResultSegmentStatus_OK, valdr, false, new RenderContextMask());
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT, new RenderContextMask());
	}

}

} // namespace torasu::tstd
