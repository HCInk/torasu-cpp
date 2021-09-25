#include "../include/torasu/std/Rrctx_value.hpp"

namespace torasu::tstd {

Rrctx_value::Rrctx_value(std::string valueKey, std::string pipelineName)
	: SimpleRenderable(true, false), mapping(valueKey, pipelineName) {}

Rrctx_value::~Rrctx_value() {}

Identifier Rrctx_value::getType() {
	return "STD::RRCTX_VALUE";
}

torasu::ResultSegment* Rrctx_value::render(torasu::RenderInstruction* ri) {
	if (ri->getResultSettings()->getPipeline().str == mapping.getB()) {

		auto* rctx = ri->getRenderContext();

		auto rctxKey = mapping.getA();
		auto found = rctx->find(rctxKey);
		auto* foundData = found->second;

		auto* resultMask = new RenderContextMask();
		(*resultMask->maskMap)[rctxKey] = new DataResourceMask::DataResourceMaskSingle(foundData != nullptr ? foundData->clone() : nullptr);

		if (found != rctx->end() && foundData != nullptr) {
			return new ResultSegment(ResultSegmentStatus_OK, foundData->clone(), true, resultMask);
		} else {
			return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT, resultMask);
		}

	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::DataResource* Rrctx_value::getData() {
	return &mapping;
}

void Rrctx_value::setData(torasu::DataResource* newData) {
	if (auto* castedData = dynamic_cast<Dstring_pair*>(newData)) {
		mapping = *castedData;
		delete newData;
	} else {
		throw std::invalid_argument("The data-type \"Dstring_pair\" is only allowed");
	}
}

} // namespace torasu::tstd
