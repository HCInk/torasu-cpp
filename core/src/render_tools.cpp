#include "../include/torasu/render_tools.hpp"

using namespace std;

namespace torasu::tools {

RenderInstructionBuilder::RenderInstructionBuilder() {
	segments = new vector<ResultSegmentSettings*>();
}

RenderInstructionBuilder::~RenderInstructionBuilder() {
	for (ResultSegmentSettings* segment : *segments) {
		delete segment;
	}

	delete segments;

	if (rs != NULL) {
		delete rs;
	}
}

void RenderInstructionBuilder::buildResultSettings() {
	if (rs != NULL) {
		delete rs;
	}

	rs = new ResultSettings();

	for (ResultSegmentSettings* segment : *segments) {
		rs->push_back(segment);
	}

}

//
// Render Helper
//

RenderHelper::RenderHelper(RenderInstruction* ri)
	: ei(ri->getExecutionInterface()), li(ri->getLogInstruction()), rctx(ri->getRenderContext()), resMask(new RenderContextMask()), lrib(li) {}

RenderHelper::RenderHelper(ExecutionInterface* ei, LogInstruction li, RenderContext* rctx)
	: ei(ei), li(li), rctx(rctx), resMask(new RenderContextMask()), lrib(li) {}

RenderHelper::~RenderHelper() {
	if (resMask != nullptr) delete resMask;
}

void RenderHelper::collectMask(const RenderContextMask* mask) {
	if (resMask != nullptr) {
		if (mask != nullptr) {
			resMask->mergeInto(*mask);
		} else {
			delete resMask;
			resMask = nullptr;
		}
	}
}

RenderContextMask* RenderHelper::takeResMask() {
	RenderContextMask* mask;

	mask = resMask;
	if (mask != nullptr) {
		resMask = nullptr;
		mask->resolveUnknownsFromRctx(rctx);
	}

	return mask;
}

ResultSegment* RenderHelper::buildResult(ResultSegmentStatus status) {
	return new ResultSegment(status, nullptr, false, takeResMask(), lrib.build());
}

ResultSegment* RenderHelper::buildResult(DataResource* dr, ResultSegmentStatus status) {
	if (status == ResultSegmentStatus_OK && lrib.hasError)
		status = ResultSegmentStatus_OK_WARN;

	return new ResultSegment(status, dr, true, takeResMask(), lrib.build());
}


} // namespace torasu::tools