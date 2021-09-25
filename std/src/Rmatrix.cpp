#include "../include/torasu/std/Rmatrix.hpp"

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

Rmatrix::Rmatrix(std::initializer_list<torasu::tstd::NumSlot> numbers, size_t height)
	: SimpleRenderable(true, true), height(height) {

	size_t i = 0;
	for (torasu::tstd::NumSlot num : numbers) {
		vals[i++] = num;
	}

}

Rmatrix::~Rmatrix() {}

Identifier Rmatrix::getType() {
	return "STD::RMATRIX";
}

DataResource* Rmatrix::getData() {
	return &height;
}

void Rmatrix::setData(DataResource* data) {
	if (torasu::tstd::Dnum* num = dynamic_cast<torasu::tstd::Dnum*>(data)) {
		height = *num;
	} else {
		throw std::invalid_argument("The data-type \"Dnum\" is only allowed");
	}
}

torasu::ElementMap Rmatrix::getElements() {
	torasu::ElementMap elements;

	for (auto& elem : vals) {
		elements["n" + std::to_string(elem.first)] = elem.second.get();
	}

	return elements;
}

void Rmatrix::setElement(std::string key, Element* elem) {

	if (key.length() >= 2 && key.substr(0, 1) == "n") {

		std::string numStr = key.substr(1);

		size_t index;
		try {
			index = std::stoul(numStr);
		} catch (std::invalid_argument& ex) {
			throw torasu::tools::makeExceptSlotDoesntExist(key);
		}

		if (Renderable* rnd = dynamic_cast<Renderable*>(elem)) {
			vals[index] = rnd;
		} else {
			throw torasu::tools::makeExceptSlotOnlyRenderables(key);
		}
	}
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

RenderResult* Rmatrix::render(RenderInstruction* ri) {

	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_VEC) {

		torasu::tools::RenderHelper rh(ri);

		ResultSettings numSetting(TORASU_STD_PL_NUM, nullptr);

		std::map<size_t, torasu::ExecutionInterface::ResultPair*> resultMap;
		std::vector<torasu::ExecutionInterface::ResultPair> pairVec(vals.size());

		torasu::ExecutionInterface::ResultPair* rpPtr = pairVec.data();

		for (auto& slot : vals) {
			torasu::Renderable* rnd = slot.second.get();
			auto id = rh.enqueueRender(rnd, &numSetting);
			(*rpPtr) = {id, nullptr};
			resultMap[slot.first] = rpPtr;
			rpPtr++;
		}

		rpPtr = pairVec.data();
		rh.ei->fetchRenderResults(rpPtr, vals.size());

		size_t highestNum = resultMap.rbegin()->first;

		size_t height = static_cast<size_t>(this->height.getNum());
		size_t width = highestNum/height + 1;

		torasu::tstd::Dmatrix matrix(width, height);
		torasu::tstd::Dnum* numArr = matrix.getNums();

		for (auto result : resultMap) {
			RenderResult* rr = result.second->result;
			auto val = rh.evalResult<torasu::tstd::Dnum>(rr);
			if (!val) continue;
			numArr[result.first] = *val.getResult();
			delete rr;
		}

		return rh.buildResult(new torasu::tstd::Dmatrix(matrix));
	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT, new RenderContextMask());
	}

}

} // namespace torasu::tstd
