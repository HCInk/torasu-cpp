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

torasu::ResultSegment* Rboilerplate::render(torasu::RenderInstruction* ri) {
	std::string pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_NUM) {
		tools::RenderHelper rh(ri);

		torasu::ResultSettings rs(TORASU_STD_PL_NUM, nullptr);

		auto renderId = rh.enqueueRender(exampleRnd, &rs);

		std::unique_ptr<torasu::ResultSegment> rndRes(rh.fetchRenderResult(renderId));

		auto fetchedRes = rh.evalResult<tstd::Dnum>(rndRes.get());

		double num = fetchedRes.getResult() != nullptr ? fetchedRes.getResult()->getNum() : 0;

		// Processing

		num *= data->getNum();

		auto* result = new torasu::tstd::Dnum(num);

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, result, true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rboilerplate::getElements() {
	torasu::ElementMap elems;
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
		delete this->data;
		this->data = castedData;
	} else {
		throw std::invalid_argument("The data-type \"Dboilerplate\" is only allowed");
	}
}

} // namespace torasu::texample
