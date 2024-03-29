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

torasu::RenderResult* Rfloor_mod::render(torasu::RenderInstruction* ri) {
	tools::RenderHelper rh(ri);
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);

		auto valRid = rh.enqueueRender(valRnd, &resSetting);
		auto facRid = rh.enqueueRender(facRnd, &resSetting);

		std::unique_ptr<torasu::RenderResult> valRes(rh.fetchRenderResult(valRid));
		std::unique_ptr<torasu::RenderResult> facRes(rh.fetchRenderResult(facRid));

		auto* fetchedVal = rh.evalResult<torasu::tstd::Dnum>(valRes.get()).getResult();
		auto* fetchedFac = rh.evalResult<torasu::tstd::Dnum>(facRes.get()).getResult();

		if (fetchedVal == nullptr) {
			return rh.buildResult(new torasu::tstd::Dnum(0), torasu::RenderResultStatus_OK_WARN);
		}

		if (fetchedFac == nullptr) {
			return rh.buildResult(new torasu::tstd::Dnum(*fetchedVal), torasu::RenderResultStatus_OK_WARN);
		}

		double val = fetchedVal->getNum();
		double fac = fetchedFac->getNum();

		double res = val - fac * floor(val/fac);

		return rh.buildResult(new torasu::tstd::Dnum(res));
	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rfloor_mod::getElements() {
	torasu::ElementMap elems;
	elems["val"] = valRnd;
	elems["fac"] = facRnd;
	return elems;
}

const torasu::OptElementSlot Rfloor_mod::setElement(std::string key, const torasu::ElementSlot* elem) {
	if (key == "val") return torasu::tools::trySetRenderableSlot(&valRnd, elem);
	if (key == "fac") return torasu::tools::trySetRenderableSlot(&facRnd, elem);
	return nullptr;
}

} // namespace torasu::tstd