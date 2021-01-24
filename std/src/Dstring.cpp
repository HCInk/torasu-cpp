#include "../include/torasu/std/Dstring.hpp"

#include <string>

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

Dstring::Dstring(std::string jsonStripped, bool json) : DataPackable(jsonStripped) {}
Dstring::Dstring(torasu::json jsonParsed, bool json) : DataPackable(jsonParsed) {}

Dstring::Dstring(std::string str) {
	this->string = str;
}

const std::string& Dstring::getString() {
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

torasu::json Dstring::makeJson() {
	return string;
}

Dstring* Dstring::clone() {
	return new Dstring(*this);
}

} // namespace torasu::tstd