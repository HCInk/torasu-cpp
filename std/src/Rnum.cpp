#include "../include/torasu/std/Rnum.hpp"

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

using namespace std;

namespace {
auto IDENT = "STD::RNUM";
} // namespace

namespace torasu::tstd {

Rnum::Rnum(double val) : SimpleRenderable(true, false) {
	valdr = new Dnum(val);
}

Rnum::Rnum(Dnum val) : SimpleRenderable(true, false) {
	valdr = new Dnum(val);
}

Rnum::~Rnum() {
	delete valdr;
}

Identifier Rnum::getType() {
	return IDENT;
}

DataResource* Rnum::getData() {
	return valdr;
}

void Rnum::setData(DataResource* data) {
	if (Dnum* dpnum = dynamic_cast<Dnum*>(data)) {
		delete valdr;
		valdr = dpnum;
	} else {
		throw invalid_argument("The data-type \"DNum\" is only allowed");
	}
}

RenderResult* Rnum::render(RenderInstruction* ri) {

	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {
		return new RenderResult(RenderResultStatus_OK, valdr, false, new RenderContextMask());
	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT, new RenderContextMask());
	}

}

namespace {

static class : public torasu::ElementFactory {
	torasu::Identifier getType() const override {
		return IDENT;
	}

	torasu::UserLabel getLabel() const override {
		return {
		name: "Number"
			,
		description: "A defines number-value"
		};
	}

	torasu::Element* create(torasu::DataResource** data, const torasu::ElementMap& elements) const override {
		std::unique_ptr<Rnum> elem(new Rnum(0));
		if (data != nullptr) {
			elem->setData(*data);
			*data = nullptr;
		}
		return elem.release();
	}

	SlotIndex getSlotIndex() const override {
		return {slotIndex: nullptr, slotCount: 0};
	}
} FACTORY_INSTANCE;

} // namespace

const torasu::ElementFactory* const Rnum::FACTORY = &FACTORY_INSTANCE;

} // namespace torasu::tstd
