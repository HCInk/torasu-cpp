#include "../include/torasu/std/Rjson_prop.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Dfile.hpp>
#include <torasu/std/Dnum.hpp>

namespace {

inline std::vector<std::string> split(std::string base, std::string delimiter) {
	std::vector<std::string> final;
	size_t pos = 0;
	std::string token;
	while ((pos = base.find(delimiter)) != std::string::npos) {
		token = base.substr(0, pos);
		final.push_back(token);
		base.erase(0, pos + delimiter.length());
	}
	final.push_back(base);
	return final;
}

inline std::string combine(std::vector<std::string> list, size_t begin, size_t end, std::string delimiter) {
	std::string str;
	size_t i = begin;

	while (true) {
		str += list[i];
		i++;
		if (i <= end) {
			str += delimiter;
		} else {
			break;
		}
	}

	return str;
}

} // namespace

namespace torasu::tstd {

Rjson_prop::Rjson_prop(std::string path, torasu::tools::RenderableSlot jsonRnd)
	: SimpleRenderable("STD::RJSON_PROP", true, true),
	  path(new torasu::tstd::Dstring(path)), jsonRnd(jsonRnd) {}


Rjson_prop::~Rjson_prop() {
	delete path;
}

torasu::ResultSegment* Rjson_prop::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING || pipeline == TORASU_STD_PL_NUM) {

		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();

		// Sub-renderings

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dfile>(TORASU_STD_PL_FILE, nullptr);

		auto renderId = rib.enqueueRender(jsonRnd, rctx, ei);

		std::unique_ptr<torasu::RenderResult> rndRes(ei->fetchRenderResult(renderId));

		auto fetchedRes = segHandle.getFrom(rndRes.get());

		torasu::tstd::Dfile* srcJson = fetchedRes.getResult();

		if (srcJson == nullptr) {
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_INTERNAL_ERROR);
		}

		char* dataPtr = reinterpret_cast<char*>(srcJson->getFileData());

		auto json = torasu::json::parse(std::string(dataPtr, srcJson->getFileSize()));

		std::string pathStr = this->path->getString();

		if (pathStr.length() <= 0) {
			throw std::logic_error("Path sting may not be empty!");
		}

		std::vector<std::string> path = split(pathStr, ".");


		for (size_t i = 0; i < path.size(); i++) {
			if (!json.is_object()) {
				if (i == 0) {
					throw std::runtime_error("Root is not an object! - Dump at path: \n" + json.dump());
				} else {
					throw std::runtime_error("\"" + combine(path, 0, i-1, ".") + "\" is not an object! - Dump at path: \n" + json.dump());
				}
			}

			json = json[path[i]];
		}

		if (pipeline == TORASU_STD_PL_STRING) {
			if (!json.is_string()) {
				throw std::runtime_error("\"" + combine(path, 0, path.size(), ".") + "\" is not a string! - Dump at path: \n" + json.dump());
			}
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dstring(json), true);
		} else if (pipeline == TORASU_STD_PL_NUM) {
			if (!json.is_number()) {
				throw std::runtime_error("\"" + combine(path, 0, path.size(), ".") + "\" is not a number! - Dump at path: \n" + json.dump());
			}
			return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, new torasu::tstd::Dnum(json), true);
		} else {
			throw std::logic_error("Unexpected branching, this is a bug.");
		}

	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rjson_prop::getElements() {
	torasu::ElementMap elems;

	elems["json"] = jsonRnd.get();

	return elems;
}

void Rjson_prop::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("json", &jsonRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

torasu::DataResource* Rjson_prop::getData() {
	return path;
}

void Rjson_prop::setData(torasu::DataResource* data) {
	if (auto* castedData = dynamic_cast<torasu::tstd::Dstring*>(data)) {
		delete data;
		data = castedData;
	} else {
		throw std::invalid_argument("The data-type \"Dstring\" is only allowed");
	}
}

} // namespace torasu::tstd
