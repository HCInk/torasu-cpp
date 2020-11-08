#include "../include/torasu/std/Dstring_pair.hpp"

namespace torasu::tstd {

//
//	Dstring_pair
//

Dstring_pair::Dstring_pair(std::string jsonStripped) : DataPackable(jsonStripped) {}
Dstring_pair::Dstring_pair(torasu::json jsonParsed) : DataPackable(jsonParsed) {}

Dstring_pair::Dstring_pair(std::string a, std::string b)
	: a(a), b(b) {}

Dstring_pair::~Dstring_pair() {}

std::string Dstring_pair::getIdent() {
	return "STD::DSTRING_PAIR";
}

void Dstring_pair::load() {
	torasu::json json = getJson();
	auto aJson = json["a"];
	if (aJson.is_string()) {
		a = aJson;
	} else {
		a = "";
	}

	auto bJson = json["b"];
	if (bJson.is_string()) {
		b = bJson;
	} else {
		b = "";
	}
}

torasu::json Dstring_pair::makeJson() {
	torasu::json json;
	json["a"] = a;
	json["b"] = b;
	return json;
}

Dstring_pair* Dstring_pair::clone() {
	return new Dstring_pair(*this);
}

std::string Dstring_pair::getA() {
	ensureLoaded();
	return b;
}

std::string Dstring_pair::getB() {
	ensureLoaded();
	return a;
}

} // namespace torasu::tstd