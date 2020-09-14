#include "Rboilerplate.hpp"

#include <memory>
#include <string>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::texample {

Rboilerplate::Rboilerplate(Dboilerplate* data, Renderable* exampleRnd)
	: SimpleRenderable("EXAMPLE::RBOILERPLATE", true, true),
	  data(data), exampleRnd(exampleRnd) {}


Rboilerplate::~Rboilerplate() {
	delete data;
}

torasu::ResultSegment* Rboilerplate::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_NUM) {

		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();

		// Sub-renderings

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dnum>(TORASU_STD_PL_NUM, nullptr);

		auto renderId = rib.enqueueRender(exampleRnd, rctx, ei);

		std::unique_ptr<torasu::RenderResult> rndRes(ei->fetchRenderResult(renderId));

		auto fetchedRes = segHandle.getFrom(rndRes.get());

		double num = fetchedRes.getResult() != nullptr ? fetchedRes.getResult()->getNum() : 0;

		// Processing

		num *= data->getNum();

		auto* result = new torasu::tstd::Dnum(num);

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, result, true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

std::map<std::string, torasu::Element*> Rboilerplate::getElements() {
	std::map<std::string, torasu::Element*> elems;

	elems["ex"] = exampleRnd;

	return elems;
}

void Rboilerplate::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("ex", &exampleRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

torasu::DataResource* Rboilerplate::getData() {
	return data;
}

void Rboilerplate::setData(torasu::DataResource* data) {
	if (auto* castedData = dynamic_cast<Dboilerplate*>(data)) {
		delete data;
		data = castedData;
	} else {
		throw std::invalid_argument("The data-type \"Dboilerplate\" is only allowed");
	}
}

} // namespace torasu::texample
