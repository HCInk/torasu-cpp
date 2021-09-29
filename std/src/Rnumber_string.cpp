#include "../include/torasu/std/Rnumber_string.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>

namespace torasu::tstd {

Rnumber_string::Rnumber_string(torasu::tstd::NumSlot src)
	: SimpleRenderable(false, true), srcRnd(src) {}

Rnumber_string::~Rnumber_string() {}

Identifier Rnumber_string::getType() {
	return "STD::RNUM_STR";
}

torasu::RenderResult* Rnumber_string::render(torasu::RenderInstruction* ri) {
	auto pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {
		tools::RenderHelper rh(ri);

		torasu::ResultSettings strSetting(TORASU_STD_PL_NUM, nullptr);
		std::unique_ptr<RenderResult> rr(rh.runRender(srcRnd, &strSetting));

		auto res = rh.evalResult<tstd::Dnum>(rr.get());

		if (!res) {
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide source for number-string.", res.takeInfoTag());

			return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
		}

		return rh.buildResult(new tstd::Dstring(std::to_string(res.getResult()->getNum())));

	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rnumber_string::getElements() {
	torasu::ElementMap elemMap;

	elemMap["src"] = srcRnd.get();

	return elemMap;
}

void Rnumber_string::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("src", &srcRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd