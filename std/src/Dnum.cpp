#include "../include/torasu/std/Dnum.hpp"

#include <string>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace torasu::tstd {

Dnum::Dnum(string jsonStripped) : DataPackable(jsonStripped) {}
Dnum::Dnum(json jsonParsed) : DataPackable(jsonParsed) {}

Dnum::Dnum(double num) {
	this->num = num;
}

double Dnum::getNum() {
	ensureLoaded();
	return num;
}

std::string Dnum::getIdent() {
	return ident;
}

void Dnum::load() {
	json json = getJson();
	if (json.is_number()) {
		num = json;
	} else {
		num = 0;
	}
}

nlohmann::json Dnum::makeJson() {
	return num;
}

} // namespace torasu::tstd
