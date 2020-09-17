#include "../include/torasu/std/Rproperty.hpp"

#include <torasu/RenderableProperties.hpp>
#include <torasu/render_tools.hpp>

namespace torasu::tstd {

//
// TODO Format support
//

Rproperty::Rproperty(Renderable* propertySrc, std::string fromProperty, std::string servedPipeline)
	: SimpleRenderable("STD::RPROPERTY", false, true),
	  propertySrc(propertySrc), fromProperty(fromProperty), servedPipeline(servedPipeline) {}

Rproperty::~Rproperty() {}

torasu::ResultSegment* Rproperty::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	if (resSettings->getPipeline() == servedPipeline) {
		torasu::RenderableProperties* rp = torasu::tools::getProperties(propertySrc, { fromProperty }, ri->getExecutionInterface(), ri->getRenderContext());
		torasu::DataResourceHolder holder = (*rp)[fromProperty];
		bool owns = holder.owns();
		torasu::DataResource* value = owns ? holder.eject() : holder.get();
		delete rp;
		if (value == nullptr) {
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
		}

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, value, owns);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rproperty::getElements() {
	torasu::ElementMap elems;

	elems["psrc"] = propertySrc;

	return elems;
}

void Rproperty::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("psrc", &propertySrc, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd