#include "../include/torasu/std/Dnum.hpp"

#include <string>

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

Dnum::Dnum(string jsonStripped) : DataPackable(jsonStripped) {}
Dnum::Dnum(json jsonParsed) : DataPackable(jsonParsed) {}

Dnum::Dnum(double num) : num(num) {}

Dnum::~Dnum() {}

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

torasu::json Dnum::makeJson() {
	return num;
}

Dnum* Dnum::clone() {
	return new Dnum(*this);
}

} // namespace torasu::tstd
