#include "../include/torasu/std/Rstring.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace {
auto IDENT = "STD::RSTRING";
} // namespace

namespace torasu::tstd {


Rstring::Rstring(std::string str)
	: SimpleRenderable(true, false),
	  str(new torasu::tstd::Dstring(str)) {}


Rstring::~Rstring() {
	delete str;
}

Identifier Rstring::getType() {
	return IDENT;
}

torasu::RenderResult* Rstring::render(torasu::RenderInstruction* ri) {
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_STRING) {
		return new torasu::RenderResult(torasu::RenderResultStatus_OK, str, false, new RenderContextMask());
	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT, new RenderContextMask());
	}
}

torasu::DataResource* Rstring::getData() {
	return str;
}

void Rstring::setData(torasu::DataResource* data) {
	if (auto* castedData = dynamic_cast<torasu::tstd::Dstring*>(data)) {
		delete str;
		str = castedData;
	} else {
		throw std::invalid_argument("The data-type \"Dstring\" is only allowed");
	}
}

namespace {

static class : public torasu::ElementFactory {
	torasu::Identifier getType() const override {
		return IDENT;
	}

	torasu::UserLabel getLabel() const override {
		return {
			.name = "Plain-Text",
			.description = "A defines text-value"
		};
	}

	torasu::Element* create(torasu::DataResource** data, const torasu::ElementMap& elements) const override {
		std::unique_ptr<Rstring> elem(new Rstring(""));
		if (data != nullptr) {
			elem->setData(*data);
			*data = nullptr;
		}
		return elem.release();
	}

	SlotIndex getSlotIndex() const override {
		return {.slotIndex = nullptr, .slotCount = 0};
	}
} FACTORY_INSTANCE;

} // namespace

const torasu::ElementFactory* const Rstring::FACTORY = &FACTORY_INSTANCE;

} // namespace torasu::tstd