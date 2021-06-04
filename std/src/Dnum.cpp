#include "../include/torasu/std/Dnum.hpp"

#include <string>

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

Dnum::Dnum() : num(0) {}
Dnum::Dnum(double num) : num(num) {}

Dnum::Dnum(string jsonStripped) : DataPackable(jsonStripped) {}
Dnum::Dnum(json jsonParsed) : DataPackable(jsonParsed) {}

void Dnum::operator=(Dnum value) {
	setUpdate();
	this->num = value.getNum();
}

void Dnum::operator=(double value) {
	setUpdate();
	this->num = value;
}

Dnum::~Dnum() {}

double Dnum::getNum() {
	ensureLoaded();
	return num;
}

std::string Dnum::getIdent() const {
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

Dnum* Dnum::clone() const {
	return new Dnum(*this);
}

} // namespace torasu::tstd
