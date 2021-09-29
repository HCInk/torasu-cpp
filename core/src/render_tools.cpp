#include "../include/torasu/render_tools.hpp"

using namespace std;

namespace torasu::tools {

//
// Render Helper
//

RenderHelper::RenderHelper(RenderInstruction* ri)
	: ei(ri->getExecutionInterface()), li(ri->getLogInstruction()), rctx(ri->getRenderContext()), rs(ri->getResultSettings()), resMask(new RenderContextMask()), lrib(li) {}

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

RenderResult* RenderHelper::buildResult(RenderResultStatus status) {
	return new RenderResult(status, nullptr, false, takeResMask(), lrib.build());
}

RenderResult* RenderHelper::buildResult(DataResource* dr, RenderResultStatus status) {
	if (status == RenderResultStatus_OK && lrib.hasError)
		status = RenderResultStatus_OK_WARN;

	return new RenderResult(status, dr, true, takeResMask(), lrib.build());
}


} // namespace torasu::tools