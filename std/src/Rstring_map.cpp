#include "../include/torasu/std/Rstring_map.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring_map.hpp>

namespace torasu::tstd {


Rstring_map::Rstring_map(std::initializer_list<MapPair> mapping)
	: SimpleRenderable("STD::RSTR_MAP", false, true) {
	for (auto entry : mapping) {
		map[entry.key] = entry.slot;
	}
}

Rstring_map::~Rstring_map() {}

torasu::ResultSegment* Rstring_map::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_MAP) {
		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();
		auto li = ri->getLogInstruction();

		torasu::tools::RenderInstructionBuilder rib;
		auto handle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		size_t resCount = map.size();
		std::vector<std::string> keyArr(resCount);
		std::vector<torasu::ExecutionInterface::ResultPair> rpArr(resCount);

		size_t i = 0;
		for (auto& entry : map) {
			keyArr[i] = entry.first;
			size_t renderId = rib.enqueueRender(entry.second.get(), rctx, ei, li);
			rpArr[i] = {renderId};
			i++;
		}

		ei->fetchRenderResults(rpArr.data(), resCount);

		auto* result = new torasu::tstd::Dstring_map();

		for (size_t i = 0; i < resCount; i++) {
			std::unique_ptr<RenderResult> rr(rpArr[i].result);
			auto* str = handle.getFrom(rr.get()).getResult();
			// TODO Handle exceptions
			result->set(keyArr[i], str->getString());
		}

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, result, true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_map::getElements() {
	torasu::ElementMap elemMap;

	for (auto mapping : map) {
		elemMap[mapping.first] = mapping.second.get();
	}

	return elemMap;
}

void Rstring_map::setElement(std::string key, Element* elem) {
	if (Renderable* rnd = dynamic_cast<Renderable*>(elem)) {
		map[key] = rnd;
	} else {
		throw torasu::tools::makeExceptSlotOnlyRenderables(key);
	}
}

} // namespace torasu::tstd