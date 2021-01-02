#include "../include/torasu/std/Rsin.hpp"

#include <string>
#include <optional>
#include <chrono>
#include <cmath>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/std/pipeline_names.hpp>

using namespace std;

namespace torasu::tstd {

Rsin::Rsin(NumSlot val)
	: SimpleRenderable(std::string("STD::RSIN"), false, true), valRnd(val) {}

Rsin::~Rsin() {

}

ResultSegment* Rsin::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline() == TORASU_STD_PL_NUM) {

		tools::RenderInstructionBuilder rib;
		tools::RenderResultSegmentHandle<Dnum> resHandle = rib.addSegmentWithHandle<Dnum>(TORASU_STD_PL_NUM, NULL);

		// Sub-Renderings
		auto ei = ri->getExecutionInterface();
		auto li = ri->getLogInstruction();
		auto rctx = ri->getRenderContext();

		auto rid = rib.enqueueRender(valRnd, rctx, ei, li);

		std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rid));

		// Calculating Result from Results

		std::optional<double> calcResult;

		tools::CastedRenderSegmentResult<Dnum> val = resHandle.getFrom(rr.get());

		if (val.getResult()!=nullptr) {
			calcResult = sin(val.getResult()->getNum() * M_PI / 180);
		}

		// Saving Result

		if (calcResult.has_value()) {
			Dnum* mulRes = new Dnum(calcResult.value());
			return new ResultSegment(ResultSegmentStatus_OK, mulRes, true);
		} else {
			Dnum* errRes = new Dnum(0);
			return new ResultSegment(ResultSegmentStatus_OK_WARN, errRes, true);
		}

	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}

}

map<string, Element*> Rsin::getElements() {
	map<string, Element*> elems;

	elems["v"] = valRnd.get();
	return elems;
}

void Rsin::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("v", &valRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd