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

torasu::ResultSegment* Rstring_concat::render(torasu::RenderInstruction* ri) {
	tools::RenderHelper rh(ri);
	std::string pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING) {

		torasu::ResultSettings stringType(TORASU_STD_PL_STRING, nullptr);
		torasu::ResultSettings mapType(TORASU_STD_PL_MAP, nullptr);

		std::unique_ptr<torasu::ResultSegment> mapRes(rh.runRender(listRnd, &mapType));

		auto castedMap = rh.evalResult<tstd::Dstring_map>(mapRes.get());
		if (!castedMap) {
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::LogLevel::WARN, "Failed to generate string, since list could not be retrieved.", castedMap.takeInfoTag());
			return rh.buildResult(new tstd::Dstring(""), torasu::ResultSegmentStatus_OK_WARN);
		}

		std::vector<std::unique_ptr<torasu::RenderContext>> contexts;
		std::vector<std::unique_ptr<torasu::tstd::Dstring>> params;
		std::vector<torasu::ExecutionInterface::ResultPair> rpList;

		auto* map = castedMap.getResult();

		for (size_t i = 0;; i++) {
			const std::string* value = map->get(std::to_string(i));
			if (value == nullptr) break;
			auto* modRctx = new auto(*rh.rctx);
			auto* valData = new torasu::tstd::Dstring(*value);
			contexts.emplace_back() = std::unique_ptr<torasu::RenderContext>(modRctx);
			params.emplace_back() = std::unique_ptr<torasu::tstd::Dstring>(valData);
			(*modRctx)[RCTX_KEY_VALUE] = valData;

			auto rid = rh.enqueueRender(genRnd, &stringType, modRctx);

			rpList.push_back({rid});
		}

		rh.ei->fetchRenderResults(rpList.data(), rpList.size());

		std::string resStr;

		for (size_t i = 0; i < rpList.size(); i++) {
			auto render = rpList[i];
			std::unique_ptr<torasu::ResultSegment> rr(render.result);
			auto res = rh.evalResult<tstd::Dstring>(rr.get());

			torasu::tstd::Dstring* str = res.getResult();

			if (res) {
				resStr += str->getString();
			} else {
				if (rh.mayLog(torasu::LogLevel::WARN))
					rh.lrib.logCause(torasu::LogLevel::WARN,
									 "Concat-generator returned no result for entry #" + std::to_string(i) +
									 " - Given param: \"" + params[i]->getString() + "\"",
									 res.takeInfoTag());
			}
		}

		contexts.clear();
		params.clear();


		return rh.buildResult(new torasu::tstd::Dstring(resStr));
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