#include "../include/torasu/std/Rstring.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {


Rstring::Rstring(std::string str)
	: SimpleRenderable("STD::RSTRING", true, false),
	  str(new torasu::tstd::Dstring(str)) {}


Rstring::~Rstring() {
	delete str;
}

torasu::ResultSegment* Rstring::render(torasu::RenderInstruction* ri) {
	std::string pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, str, false, new RenderContextMask());
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT, new RenderContextMask());
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

} // namespace torasu::tstd