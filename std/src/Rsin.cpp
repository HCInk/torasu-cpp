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

Rsin::Rsin(NumSlot val) : SimpleRenderable(false, true), valRnd(val) {}

Rsin::~Rsin() {}

Identifier Rsin::getType() {
	return "STD::RSIN";
}

RenderResult* Rsin::render(RenderInstruction* ri) {
	tools::RenderHelper rh(ri);
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_NUM) {

		torasu::ResultSettings resSetting(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
		std::unique_ptr<torasu::RenderResult> rr(rh.runRender(valRnd, &resSetting));
		auto val = rh.evalResult<tstd::Dnum>(rr.get());

		if (val) {
			double calcResult = sin(val.getResult()->getNum() * M_PI / 180);
			return rh.buildResult(new tstd::Dnum(calcResult));
		} else {
			if (rh.mayLog(WARN)) {
				rh.lrib.logCause(WARN, "Failed to render parameter of sinus, returning 0", val.takeInfoTag());
			}
			return rh.buildResult(new tstd::Dnum(0), torasu::RenderResultStatus_OK_WARN);
		}

	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT);
	}

}

torasu::ElementMap Rsin::getElements() {
	torasu::ElementMap elems;
	elems["v"] = valRnd;
	return elems;
}

const torasu::OptElementSlot Rsin::setElement(std::string key, const torasu::ElementSlot* elem) {
	if (key == "v") return torasu::tools::trySetRenderableSlot(&valRnd, elem);
	return nullptr;
}

} // namespace torasu::tstd