#include "../include/torasu/std/Rjson_prop.hpp"

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Dfile.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Dstring_map.hpp>

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

inline std::string jsonToStr(const torasu::json& json) {
	if (json.is_string()) {
		return json;
	} else {
		return json.dump();
	}
}

} // namespace

namespace torasu::tstd {

Rjson_prop::Rjson_prop(std::string path, torasu::RenderableSlot jsonRnd, bool optional)
	: SimpleRenderable(true, true),
	  config(new torasu::tstd::Dstring_pair(path, optional ? "opt" : "def")),
	  jsonRnd(jsonRnd) {}


Rjson_prop::~Rjson_prop() {}

Identifier Rjson_prop::getType() {
	return "STD::RJSON_PROP";
}

torasu::RenderResult* Rjson_prop::render(torasu::RenderInstruction* ri) {
	auto pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_STRING || pipeline == TORASU_STD_PL_NUM || pipeline == TORASU_STD_PL_MAP) {

		tools::RenderHelper rh(ri);

		//
		// Load input/parameters
		//

		torasu::ResultSettings fileSetting(TORASU_STD_PL_FILE, torasu::tools::NO_FORMAT);
		auto renderId = rh.enqueueRender(jsonRnd, &fileSetting);

		std::unique_ptr<torasu::RenderResult> rndRes(rh.fetchRenderResult(renderId));

		auto fetchedRes = rh.evalResult<torasu::tstd::Dfile>(rndRes.get());


		if (!fetchedRes) {
			rh.lrib.logCause(ERROR, "Failed to get source-json!", fetchedRes.takeInfoTag());
			return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
		}

		torasu::tstd::Dfile* srcJson = fetchedRes.getResult();

		char* dataPtr = reinterpret_cast<char*>(srcJson->getFileData());

		if (rh.mayLog(torasu::LogLevel::DEBUG))
			rh.li.logger->log(torasu::LogLevel::DEBUG, "Parsing: " + std::string(dataPtr, srcJson->getFileSize()));

		auto json = torasu::json::parse(std::string(dataPtr, srcJson->getFileSize()));

		std::string pathStr = this->config->getA();

		if (pathStr.empty()) {
			throw std::logic_error("Path sting may not be empty!");
		}

		std::vector<std::string> path = split(pathStr, ".");

		bool optional = this->config->getB() == "opt";

		//
		// Navigate to path
		//

		for (size_t i = 0; i < path.size(); i++) {
			if (json.is_object()) {
				json = json[path[i]];
			} else if (json.is_array()) {
				try {
					auto index = std::stol(path[i]);
					json = json[index];
				} catch (const std::invalid_argument& ex) {
					if (optional) { // Return invalid-segment if set to optional
						return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
					} else {
						std::string pathDesc = i == 0 ? "Root" : "\"" + combine(path, 0, i-1, ".") + "\"";
						throw std::runtime_error(pathDesc + " is an array, but the index \"" + path[i] + "\" provided in the path, can't be parsed as a number: " + ex.what());
					}
				}
			} else {
				if (optional) { // Return invalid-segment if set to optional
					return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
				} else {
					std::string pathDesc = i == 0 ? "Root" : "\"" + combine(path, 0, i-1, ".") + "\"";
					throw std::runtime_error(pathDesc + " is not an object/array! - Dump at path: \n" + json.dump());
				}

			}
		}

		//
		// Evalulate data
		//

		if (json.is_null()) {
			if (optional) { // Return invalid-segment if set to optional
				return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
			} else {
				throw std::runtime_error("\"" + combine(path, 0, path.size()-1, ".") + "\" does not exist!");
			}
		}

		if (pipeline == TORASU_STD_PL_STRING) {
			if (json.is_string()) {
				return rh.buildResult(new torasu::tstd::Dstring(json));
			} else if (json.is_number()) {
				return rh.buildResult(new torasu::tstd::Dstring(json.dump()));
			} else {
				if (optional) { // Return invalid-segment if set to optional
					return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
				} else {
					throw std::runtime_error("\"" + combine(path, 0, path.size()-1, ".") + "\" is not a string/number! - Dump at path: \n" + json.dump());
				}
			}
		} else if (pipeline == TORASU_STD_PL_NUM) {
			if (!json.is_number()) {
				if (optional) { // Return invalid-segment if set to optional
					return rh.buildResult(torasu::RenderResultStatus_INVALID_SEGMENT);
				} else {
					throw std::runtime_error("\"" + combine(path, 0, path.size()-1, ".") + "\" is not a number! - Dump at path: \n" + json.dump());
				}
			}
			return rh.buildResult(new torasu::tstd::Dnum(json));
		} else if (pipeline == TORASU_STD_PL_MAP) {
			auto* res = new torasu::tstd::Dstring_map();

			if (json.is_array()) {
				size_t i = 0;
				for (auto& item : json) {
					res->set(std::to_string(i), jsonToStr(item));
					i++;
				}
			} else if (json.is_object()) {
				for (auto it = json.begin(); it!=json.end(); it++) {
					res->set(it.key(), jsonToStr(*it));
				}
			} else {
				res->set("0", jsonToStr(json));
			}

			return rh.buildResult(res);
		} else {
			throw std::logic_error("Unexpected branching, this is a bug.");
		}

	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rjson_prop::getElements() {
	torasu::ElementMap elems;

	elems["json"] = jsonRnd;

	return elems;
}

const torasu::OptElementSlot Rjson_prop::setElement(std::string key, const torasu::ElementSlot* elem) {
	if (key == "json") return torasu::tools::trySetRenderableSlot(&jsonRnd, elem);
	return nullptr;
}

torasu::DataResource* Rjson_prop::getData() {
	return config.get();
}

void Rjson_prop::setData(torasu::DataResource* data) {
	if (auto* castedData = dynamic_cast<torasu::tstd::Dstring_pair*>(data)) {
		config = std::unique_ptr<torasu::tstd::Dstring_pair>(castedData);
	} else {
		throw std::invalid_argument("The data-type \"Dstring_pair\" is only allowed");
	}
}

} // namespace torasu::tstd
