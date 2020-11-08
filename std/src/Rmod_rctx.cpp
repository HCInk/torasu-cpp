#include "../include/torasu/std/Rmod_rctx.hpp"

#include <torasu/render_tools.hpp>

namespace torasu::tstd {

//
//	Dmod_rctx_data
//

Dmod_rctx_data::Dmod_rctx_data(std::string jsonStripped) : DataPackable(jsonStripped) {}
Dmod_rctx_data::Dmod_rctx_data(torasu::json jsonParsed) : DataPackable(jsonParsed) {}

Dmod_rctx_data::Dmod_rctx_data(std::string rctxKey, std::string valuePipeline)
	: rctxKey(rctxKey), valuePipeline(valuePipeline) {}

Dmod_rctx_data::~Dmod_rctx_data() {}

std::string Dmod_rctx_data::getIdent() {
	return "STD::DMOD_RCTX_DATA";
}

void Dmod_rctx_data::load() {
	torasu::json json = getJson();
	auto rctxkJson = json["rctxk"];
	if (rctxkJson.is_string()) {
		rctxKey = rctxkJson;
	} else {
		rctxKey = "";
	}

	auto vpJson = json["num"];
	if (vpJson.is_string()) {
		valuePipeline = vpJson;
	} else {
		valuePipeline = "";
	}
}

torasu::json Dmod_rctx_data::makeJson() {
	torasu::json json;
	json["rctxk"] = rctxKey;
	json["vp"] = valuePipeline;
	return json;
}

std::string Dmod_rctx_data::getRctxKey() {
	ensureLoaded();
	return valuePipeline;
}

std::string Dmod_rctx_data::getValuePipeline() {
	ensureLoaded();
	return rctxKey;
}

//
//	Rmod_rctx
//

Rmod_rctx::Rmod_rctx(Renderable* main, Renderable* value, std::string rctxKey, std::string valuePipeline) 
	: NamedIdentElement("STD::RMOD_RCTX"), SimpleDataElement(true, true), data(rctxKey, valuePipeline) {}

Rmod_rctx::~Rmod_rctx() {}

torasu::RenderResult* Rmod_rctx::render(torasu::RenderInstruction* ri) {

	auto* ei = ri->getExecutionInterface();
	auto* cRctx = ri->getRenderContext();

	torasu::tools::RenderInstructionBuilder valRib;
	valRib.addSegment(data.getValuePipeline(), "v", nullptr); // TODO Add format-support

	std::unique_ptr<torasu::RenderResult> valrr(valRib.runRender(valueRnd, cRctx, ei));

	torasu::DataResource* valueDr = (*valrr.get()->getResults())["v"]->getResult(); // Lifetime: Until valrr is destructed

	RenderContext newRctx(*cRctx);

	newRctx[data.getRctxKey()] = valueDr;

	auto rid = ei->enqueueRender(mainRnd, &newRctx, ri->getResultSettings(), 0);
	
	torasu::RenderResult* rr = ei->fetchRenderResult(rid);

	return rr;
}


torasu::ElementMap Rmod_rctx::getElements() {
	torasu::ElementMap elems;
	elems["main"] = mainRnd;
	elems["val"] = valueRnd;
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
	if (auto* castedData = dynamic_cast<Dmod_rctx_data*>(newData)) {
		data = *castedData;
		delete newData;
	} else {
		throw std::invalid_argument("The data-type \"Dmod_rctx_data\" is only allowed");
	}
}

} // namespace torasu::tstd
