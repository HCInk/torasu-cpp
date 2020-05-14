#include "../include/torasu/std/DPString.hpp"

#include <string>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace torasu::tstd {

DPString::DPString(std::string jsonStripped, bool json) : DataPackable(jsonStripped) {}
DPString::DPString(nlohmann::json jsonParsed, bool json) : DataPackable(jsonParsed) {}

DPString::DPString(std::string str) {
	this->string = str;
	setLoaded();
}

std::string DPString::getString() {
	ensureLoaded();
	return string;
}

std::string DPString::getIdent() {
	return ident;
}

void DPString::load() {
	json json = getJson();
	if (json.is_string()) {
		string = json;
	} else {
		string = "";
	}
}

nlohmann::json DPString::makeJson() {
	return string;
}

}