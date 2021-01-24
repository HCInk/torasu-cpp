#include "../include/torasu/std/Rstring_concat.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring_map.hpp>

namespace torasu::tstd {

const char* Rstring_concat::RCTX_KEY_VALUE = "STD::STR_CONCAT_PARAM";

Rstring_concat::Rstring_concat(torasu::tools::RenderableSlot list, torasu::tools::RenderableSlot gen)
	: SimpleRenderable("STD::RSTR_CONCAT", false, true), listRnd(list), genRnd(gen) {
}

Rstring_concat::~Rstring_concat() {}

torasu::ResultSegment* Rstring_concat::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {
		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();
		auto li = ri->getLogInstruction();

		torasu::tools::RenderInstructionBuilder mapRib;
		auto mapHandle = mapRib.addSegmentWithHandle<torasu::tstd::Dstring_map>(TORASU_STD_PL_MAP, nullptr);
		torasu::tools::RenderInstructionBuilder strRib;
		auto strHandle = strRib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		std::unique_ptr<torasu::RenderResult> mapRes(mapRib.runRender(listRnd.get(), rctx, ei, li));

		torasu::tstd::Dstring_map* map = mapHandle.getFrom(mapRes.get()).getResult();
		if (map == nullptr) {
			if (li.level <= torasu::LogLevel::ERROR)
				li.logger->log(torasu::LogLevel::ERROR, "Failed to generate string, since list could not be retrieved.");
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK_WARN, new torasu::tstd::Dstring(""), true);
		}

		std::vector<std::unique_ptr<torasu::RenderContext>> contexts;
		std::vector<std::unique_ptr<torasu::tstd::Dstring>> params;
		std::vector<torasu::ExecutionInterface::ResultPair> rpList;

		for (size_t i = 0;; i++) {
			const std::string* value = map->get(std::to_string(i));
			if (value == nullptr) break;
			auto* modRctx = new auto(*rctx);
			auto* valData = new torasu::tstd::Dstring(*value);
			contexts.emplace_back() = std::unique_ptr<torasu::RenderContext>(modRctx);
			params.emplace_back() = std::unique_ptr<torasu::tstd::Dstring>(valData);
			(*modRctx)[RCTX_KEY_VALUE] = valData;

			auto rid = strRib.enqueueRender(genRnd, modRctx, ei, li);

			rpList.push_back({rid});
		}

		ei->fetchRenderResults(rpList.data(), rpList.size());

		std::string resStr;

		for (size_t i = 0; i < rpList.size(); i++) {
			auto render = rpList[i];
			std::unique_ptr<torasu::RenderResult> rr(render.result);
			auto res = strHandle.getFrom(rr.get());

			torasu::tstd::Dstring* str = res.getResult();

			if (str != nullptr) {
				resStr += str->getString();
			} else {
				if (li.level <= torasu::LogLevel::ERROR)
					li.logger->log(torasu::LogLevel::ERROR,
								   "Concat-generator returned no result for entry #" + std::to_string(i) +
								   " - Given param: \"" + params[i]->getString() + "\"");
			}
		}

		contexts.clear();
		params.clear();


		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dstring(resStr), true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_concat::getElements() {
	torasu::ElementMap elemMap;

	elemMap["list"] = listRnd.get();
	elemMap["gen"] = genRnd.get();

	return elemMap;
}

void Rstring_concat::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("list", &listRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("gen", &genRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd