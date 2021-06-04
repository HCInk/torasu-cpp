#include "../include/torasu/std/Rmatrix.hpp"

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

Rmatrix::Rmatrix(std::initializer_list<torasu::tstd::Dnum> numbers, size_t height)
	: SimpleRenderable("STD::RMATRIX", true, false), valdr(new Dmatrix(numbers, height)) {}

Rmatrix::Rmatrix(Dmatrix val)
	: SimpleRenderable("STD::RMATRIX", true, false), valdr(new Dmatrix(val)) {}

Rmatrix::~Rmatrix() {
	delete valdr;
}

DataResource* Rmatrix::getData() {
	return valdr;
}

void Rmatrix::setData(DataResource* data) {
	if (Dmatrix* matrix = dynamic_cast<Dmatrix*>(data)) {
		delete valdr;
		valdr = matrix;
	} else {
		throw std::invalid_argument("The data-type \"Dmatrix\" is only allowed");
	}
}

ResultSegment* Rmatrix::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline() == TORASU_STD_PL_VEC) {
		return new ResultSegment(ResultSegmentStatus_OK, valdr, false, new RenderContextMask());
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT, new RenderContextMask());
	}

}

} // namespace torasu::tstd
