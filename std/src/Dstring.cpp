#include "../include/torasu/std/Dstring.hpp"

#include <string>

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

Dstring::Dstring(std::string jsonStripped, bool json) : DataPackable(jsonStripped) {
	ensureLoaded(); // XXX For now, because compare is const
}
Dstring::Dstring(torasu::json jsonParsed, bool json) : DataPackable(jsonParsed) {
	ensureLoaded(); // XXX For now, because compare is const
}

Dstring::Dstring(std::string str) {
	this->string = str;
}

const std::string& Dstring::getString() const {
	return string;
}

torasu::DataResource::CompareResult Dstring::compare(const DataResource* other) const {
	if (auto* otherStr = dynamic_cast<const Dstring*>(other)) {
		auto cmpRes = string.compare(otherStr->getString());
		if (cmpRes > 0) {
			return GREATER;
		} else if (cmpRes == 0) {
			return EQUAL;
		} else {
			return LESS;
		}
	} else {
		return TYPE_ERR;
	}
}

torasu::Identifier Dstring::getType() const {
	return "STD::DSTRING";
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

Dstring* Dstring::clone() const {
	return new Dstring(*this);
}

} // namespace torasu::tstd