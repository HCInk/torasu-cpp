#include "../include/torasu/std/Rfloor_mod.hpp"

#include <math.h>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::tstd {

Rfloor_mod::Rfloor_mod(NumSlot val, NumSlot fac)
	: SimpleRenderable("STD::RFLOOR_MOD", false, true),
	  valRnd(val), facRnd(fac) {}


Rfloor_mod::~Rfloor_mod() {}

torasu::ResultSegment* Rfloor_mod::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_NUM) {

		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();

		// Sub-renderings

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dnum>(TORASU_STD_PL_NUM, nullptr);

		auto valRid = rib.enqueueRender(valRnd, rctx, ei);
		auto facRid = rib.enqueueRender(facRnd, rctx, ei);

		std::unique_ptr<torasu::RenderResult> valRes(ei->fetchRenderResult(valRid));
		std::unique_ptr<torasu::RenderResult> facRes(ei->fetchRenderResult(facRid));

		auto* fetchedVal = segHandle.getFrom(valRes.get()).getResult();
		auto* fetchedFac = segHandle.getFrom(facRes.get()).getResult();

		if (fetchedVal == nullptr) {
			return new torasu::ResultSegment(ResultSegmentStatus_OK_WARN, new torasu::tstd::Dnum(0), true);
		}

		if (fetchedFac == nullptr) {
			return new torasu::ResultSegment(ResultSegmentStatus_OK_WARN, new torasu::tstd::Dnum(*fetchedVal), true);
		}

		double val = fetchedVal->getNum();
		double fac = fetchedFac->getNum();

		double res = val - fac * floor(val/fac);

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dnum(res), true);
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

} // namespace torasu::texample