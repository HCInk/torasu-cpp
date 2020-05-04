#include "../include/torasu/std/DPNum.hpp"

#include <string>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using namespace torasu;

namespace torasu::tstd {

DPNum::DPNum(string jsonStripped) : DataPackable(jsonStripped) {}
DPNum::DPNum(json jsonParsed) : DataPackable(jsonParsed) {}

DPNum::DPNum(double num) {
	this->num = num;
	setLoaded();
}

double DPNum::getNum() {
	ensureLoaded();
	return num;
}

std::string DPNum::getIdent() {
	return ident;
}

void DPNum::load() {
	json json = getJson();
	if (json.is_number()) {
		num = json;
	} else {
		num = 0;
	}
}

nlohmann::json DPNum::makeJson() {
	return num;
}

} // namespace torasustd
