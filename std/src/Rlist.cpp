#include "../include/torasu/std/Rlist.hpp"

#include <torasu/render_tools.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::tstd {

//
//	Rlist
//

Rlist::Rlist(std::initializer_list<tools::RenderableSlot> list, std::string rctxKey, std::string valuePipeline)
	: SimpleRenderable(true, true), data(rctxKey, valuePipeline) {
	for (auto& slot : list) {
		this->slots[length] = slot;
		length++;
	}
}

Rlist::~Rlist() {}

Identifier Rlist::getType() {
	return "STD::RLIST";
}

torasu::RenderResult* Rlist::render(torasu::RenderInstruction* ri) {
	torasu::tools::RenderHelper rh(ri);

	if (rh.matchPipeline(data.getB().c_str())) { // Length-pipeline
		return rh.buildResult(new torasu::tstd::Dnum(length));
	}

	auto foundIndex = rh.rctx->find(data.getA());

	size_t index = 0;
	if (foundIndex != rh.rctx->end()) {
		if (auto* casted = dynamic_cast<torasu::tstd::Dnum*>(foundIndex->second)) {
			index = casted->getNum();
		} else {
			if (rh.mayLog(torasu::WARN)) {
				rh.lrib.logCause(torasu::WARN,
								 "RenderContext did not contain a number (torasu::tstd::Dnum)"
								 " on selected key \"" + data.getA() + "\","
								 " will select index-0 element in list");
			}
		}
	}

	auto found = slots.find(index);
	if (found == slots.end()) {
		if (rh.mayLog(torasu::DEBUG)) {
			rh.lrib.logCause(torasu::DEBUG,
							 "Found no element at index " + std::to_string(index));
		}
		return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}

	torasu::Renderable* rnd = found->second.get();

	return rh.passRender(rnd, torasu::tools::RenderHelper::PassMode_SELECTED);
}

torasu::ElementMap Rlist::getElements() {
	torasu::ElementMap elems;
	for (auto& slot : slots) {
		elems[std::to_string(slot.first)] = slot.second.get();
	}
	return elems;
}

void Rlist::setElement(std::string key, torasu::Element* elem) {
	size_t id;
	try {
		id = std::stoul(key);
	} catch (const std::exception& ex) {
		throw torasu::tools::makeExceptSlotDoesntExist(key);
	}

	if (elem == nullptr) {
		slots[id] = nullptr;
		return;
	}

	if (auto* rnd = dynamic_cast<Renderable*>(elem)) {
		slots[id] = rnd;
	} else {
		throw torasu::tools::makeExceptSlotOnlyRenderables(key);
	}

	length = slots.rbegin()->first;
}

torasu::DataResource* Rlist::getData() {
	return &data;
}

void Rlist::setData(torasu::DataResource* newData) {
	if (auto* castedData = dynamic_cast<Dstring_pair*>(newData)) {
		data = *castedData;
		delete newData;
	} else {
		throw std::invalid_argument("The data-type \"Dstring_pair\" is only allowed");
	}
}

} // namespace torasu::tstd
