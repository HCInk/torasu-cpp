#include "../include/torasu/render_tools.hpp"

using namespace std;

namespace torasu::tools {

//
// Render Helper
//

RenderHelper::RenderHelper(RenderInstruction* ri)
	: ei(ri->getExecutionInterface()), li(ri->getLogInstruction()), rctx(ri->getRenderContext()), resMask(new RenderContextMask()), lrib(li) {}

RenderHelper::RenderHelper(ReadyInstruction* ri)
	: ei(ri->ei), li(ri->li), rctx(ri->rctx), resMask(new RenderContextMask()), lrib(li) {}

RenderHelper::RenderHelper(ExecutionInterface* ei, LogInstruction li, RenderContext* rctx, ResultSettings* rs)
	: ei(ei), li(li), rctx(rctx), rs(rs), resMask(new RenderContextMask()), lrib(li) {}

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