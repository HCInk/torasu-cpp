#include "../include/torasu/std/Rstring_map.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring_map.hpp>

namespace torasu::tstd {


Rstring_map::Rstring_map(std::initializer_list<MapPair> mapping)
	: SimpleRenderable(false, true) {
	for (auto entry : mapping) {
		map[entry.key] = entry.slot;
	}
}

Rstring_map::~Rstring_map() {}

Identifier Rstring_map::getType() {
	return "STD::RSTR_MAP";
}

torasu::RenderResult* Rstring_map::render(torasu::RenderInstruction* ri) {
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_MAP) {
		torasu::tools::RenderHelper rh(ri);
		size_t resCount = map.size();
		std::vector<std::string> keyArr(resCount);
		std::vector<torasu::ExecutionInterface::ResultPair> rpArr(resCount);

		ResultSettings rs(TORASU_STD_PL_STRING, torasu::tools::NO_FORMAT);

		size_t i = 0;
		for (auto& entry : map) {
			keyArr[i] = entry.first;
			size_t renderId = rh.enqueueRender(entry.second, &rs);
			rpArr[i] = {renderId};
			i++;
		}

		rh.fetchRenderResults(rpArr.data(), resCount);

		auto* result = new torasu::tstd::Dstring_map();

		for (size_t i = 0; i < resCount; i++) {
			std::unique_ptr<torasu::RenderResult> rr(rpArr[i].result);
			auto strRes = rh.evalResult<torasu::tstd::Dstring>(rr.get());
			if (strRes) {
				result->set(keyArr[i], strRes.getResult()->getString());
			} else {
				rh.noteSubError(strRes,
								"Failed to render entry for key \"" + keyArr[i] + "\", entry will not be included.");
			}
		}

		return rh.buildResult(result);
	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_map::getElements() {
	torasu::ElementMap elemMap;

	for (auto& mapping : map) {
		elemMap[mapping.first] = mapping.second;
	}

	return elemMap;
}

const torasu::OptElementSlot Rstring_map::setElement(std::string key, const ElementSlot* elem) {
	return torasu::tools::trySetRenderableSlot(&map, key, elem);
}

} // namespace torasu::tstd