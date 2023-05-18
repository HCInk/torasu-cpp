#include "../include/torasu/std/Rproperty.hpp"

#include <torasu/RenderableProperties.hpp>
#include <torasu/render_tools.hpp>

namespace torasu::tstd {

//
// TODO Format support
//

Rproperty::Rproperty(RenderableSlot propertySrc, std::string fromProperty, std::string servedPipeline)
	: SimpleRenderable(false, true),
	  propertySrc(propertySrc), fromProperty(fromProperty), servedPipeline(servedPipeline) {}

Rproperty::~Rproperty() {}

Identifier Rproperty::getType() {
	return "STD::RPROPERTY";
}

torasu::RenderResult* Rproperty::render(torasu::RenderInstruction* ri) {
	if (ri->getResultSettings()->getPipeline().str == servedPipeline) {
		torasu::RenderableProperties* rp = torasu::tools::getProperties(propertySrc.get(), { fromProperty }, ri->getExecutionInterface(), ri->getLogInstruction(), ri->getRenderContext());
		auto& holder = (*rp)[fromProperty];
		bool owns = holder.owns();
		torasu::DataResource* value = owns ? holder.eject() : holder.get();
		delete rp;
		if (value == nullptr) {
			return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
		}

		return new torasu::RenderResult(torasu::RenderResultStatus_OK, value, owns);
	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rproperty::getElements() {
	torasu::ElementMap elems;

	elems["psrc"] = propertySrc;

	return elems;
}

const torasu::OptElementSlot Rproperty::setElement(std::string key, const torasu::ElementSlot* elem) {
	if (key == "psrc") return torasu::tools::trySetRenderableSlot(&propertySrc, elem);
	return nullptr;
}

// TODO Value import-export

} // namespace torasu::tstd