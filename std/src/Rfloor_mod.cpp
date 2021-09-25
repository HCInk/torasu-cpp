#include "../include/torasu/std/Rfloor_mod.hpp"

#include <math.h>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::tstd {

Rfloor_mod::Rfloor_mod(NumSlot val, NumSlot fac)
	: SimpleRenderable(false, true), valRnd(val), facRnd(fac) {}


Rfloor_mod::~Rfloor_mod() {}

Identifier Rfloor_mod::getType() {
	return "STD::RFLOOR_MOD";
}

torasu::ResultSegment* Rfloor_mod::render(torasu::RenderInstruction* ri) {
	tools::RenderHelper rh(ri);
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, nullptr);

		auto valRid = rh.enqueueRender(valRnd, &resSetting);
		auto facRid = rh.enqueueRender(facRnd, &resSetting);

		std::unique_ptr<torasu::ResultSegment> valRes(rh.fetchRenderResult(valRid));
		std::unique_ptr<torasu::ResultSegment> facRes(rh.fetchRenderResult(facRid));

		auto* fetchedVal = rh.evalResult<torasu::tstd::Dnum>(valRes.get()).getResult();
		auto* fetchedFac = rh.evalResult<torasu::tstd::Dnum>(facRes.get()).getResult();

		if (fetchedVal == nullptr) {
			return rh.buildResult(new torasu::tstd::Dnum(0), torasu::ResultSegmentStatus_OK_WARN);
		}

		if (fetchedFac == nullptr) {
			return rh.buildResult(new torasu::tstd::Dnum(*fetchedVal), torasu::ResultSegmentStatus_OK_WARN);
		}

		double val = fetchedVal->getNum();
		double fac = fetchedFac->getNum();

		double res = val - fac * floor(val/fac);

		return rh.buildResult(new torasu::tstd::Dnum(res));
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rfloor_mod::getElements() {
	torasu::ElementMap elems;
	elems["val"] = valRnd.get();
	elems["fac"] = facRnd.get();
	return elems;
}

void Rfloor_mod::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("val", &valRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("fac", &facRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd