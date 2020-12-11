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

torasu::RenderResult* Rmod_rctx::render(torasu::RenderInstruction* ri) {

	auto* ei = ri->getExecutionInterface();
	auto* cRctx = ri->getRenderContext();

	torasu::tools::RenderInstructionBuilder valRib;
	valRib.addSegment(data.getB(), "v", nullptr); // TODO Add format-support

	std::unique_ptr<torasu::RenderResult> valrr(valRib.runRender(valueRnd.get(), cRctx, ei));

	torasu::DataResource* valueDr = (*valrr.get()->getResults())["v"]->getResult(); // Lifetime: Until valrr is destructed

	RenderContext newRctx(*cRctx);

	newRctx[data.getA()] = valueDr;

	auto rid = ei->enqueueRender(mainRnd.get(), &newRctx, ri->getResultSettings(), 0);

	torasu::RenderResult* rr = ei->fetchRenderResult(rid);

	return rr;
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
