#include "../include/torasu/std/Rmod_rctx.hpp"

#include <torasu/render_tools.hpp>

namespace torasu::tstd {

//
//	Rmod_rctx
//

Rmod_rctx::Rmod_rctx(tools::RenderableSlot main, tools::RenderableSlot value, std::string rctxKey, std::string valuePipeline)
	: NamedIdentElement("STD::RMOD_RCTX"), SimpleDataElement(true, true),
	  data(rctxKey, valuePipeline), mainRnd(main), valueRnd(value) {}

Rmod_rctx::~Rmod_rctx() {}

torasu::ResultSegment* Rmod_rctx::render(torasu::RenderInstruction* ri) {
	torasu::tools::RenderHelper rh(ri);

	const std::string pipeline(data.getB());
	ResultSettings sourceSettings(pipeline.c_str(), nullptr);
	std::unique_ptr<torasu::ResultSegment> valrr(rh.runRender(valueRnd.get(), &sourceSettings));
	auto valueResult = rh.evalResult<torasu::DataResource>(valrr.get(), false);

	RenderContext newRctx(*rh.rctx);
	std::string replacedValKey = data.getA();

	if (valueResult) {
		// Lifetime: Until valrr is destructed
		newRctx[replacedValKey] = valueResult.getResult();
	} else {
		if (rh.mayLog(WARN)) {
			rh.lrib.logCause(WARN,
							 "Failed to render value to replace in render-context, will leave \"" + replacedValKey + "\" unchanged.",
							 valueResult.takeInfoTag());
		}
		rh.lrib.hasError = true;
	}

	std::unique_ptr<torasu::ResultSegment> resrr(rh.runRender(mainRnd.get(), ri->getResultSettings(), &newRctx));
	auto mainResult = rh.evalResult<torasu::DataResource>(resrr.get(), false);

	torasu::ResultSegmentStatus status = mainResult.getStatus();
	std::unique_ptr<torasu::DataResource> payload(resrr->ejectOrClone());
	const RenderContextMask* payloadMask = mainResult.getResultMask();
	const RenderContextMask* valMask = valueResult.getResultMask();

	if (payloadMask != nullptr && valMask != nullptr) {
		// Both masks are valid

		std::unique_ptr<RenderContextMask> filtered(payloadMask->filter({replacedValKey}));
		std::unique_ptr<RenderContextMask> newMask(RenderContextMask::merge(*filtered, *valMask));
		filtered.reset();

		newMask->resolveUnknownsFromRctx(rh.rctx);

		rh.collectMask(newMask.get());

	} else {
		rh.collectMask(nullptr); // say that masks can't be calculated
	}


	return rh.buildResult(payload.release(), status);
}


torasu::ElementMap Rmod_rctx::getElements() {
	torasu::ElementMap elems;
	elems["main"] = mainRnd.get();
	elems["val"] = valueRnd.get();
	return elems;
}

void Rmod_rctx::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("main", &mainRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("val", &valueRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

torasu::DataResource* Rmod_rctx::getData() {
	return &data;
}

void Rmod_rctx::setData(torasu::DataResource* newData) {
	if (auto* castedData = dynamic_cast<Dstring_pair*>(newData)) {
		data = *castedData;
		delete newData;
	} else {
		throw std::invalid_argument("The data-type \"Dstring_pair\" is only allowed");
	}
}

} // namespace torasu::tstd
