#include "Dboilerplate.hpp"

#include <string>

namespace torasu::texample {

Dboilerplate::Dboilerplate(std::string jsonStripped) : DataPackable(jsonStripped) {}
Dboilerplate::Dboilerplate(torasu::json jsonParsed) : DataPackable(jsonParsed) {}

Dboilerplate::Dboilerplate(std::string str, double num)
	: str(str), num(num) {}

Dboilerplate::~Dboilerplate() {}

torasu::Identifier Dboilerplate::getType() const {
	return "EXAMPLE::DBOILERPLATE";
}

Dboilerplate* Dboilerplate::clone() const {
	return new Dboilerplate(*this);
}

void Dboilerplate::load() {
	torasu::json json = getJson();
	auto strJson = json["str"];
	if (strJson.is_string()) {
		str = strJson;
	} else {
		str = "";
	}

	auto numJson = json["num"];
	if (numJson.is_number()) {
		num = numJson;
	} else {
		num = 0;
	}
}

torasu::json Dboilerplate::makeJson() {
	torasu::json json;
	json["str"] = str;
	json["num"] = num;
	return json;
}

std::string Dboilerplate::getStr() {
	ensureLoaded();
	return str;
}

double Dboilerplate::getNum() {
	ensureLoaded();
	return num;
}

} // namespace torasu::texample
