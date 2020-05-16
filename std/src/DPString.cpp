#include "../include/torasu/std/DPString.hpp"

#include <string>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace torasu::tstd {

Dstring::Dstring(std::string jsonStripped, bool json) : DataPackable(jsonStripped) {}
Dstring::Dstring(nlohmann::json jsonParsed, bool json) : DataPackable(jsonParsed) {}

Dstring::Dstring(std::string str) {
	this->string = str;
	setLoaded();
}

std::string Dstring::getString() {
	ensureLoaded();
	return string;
}

std::string Dstring::getIdent() {
	return ident;
}

void Dstring::load() {
	json json = getJson();
	if (json.is_string()) {
		string = json;
	} else {
		string = "";
	}
}

nlohmann::json Dstring::makeJson() {
	return string;
}

} // namespace torasu::tstd