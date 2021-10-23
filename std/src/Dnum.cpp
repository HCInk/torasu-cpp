#include "../include/torasu/std/Dnum.hpp"

#include <string>

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace {
auto IDENT = "STD::DNUM";
} // namespace

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

torasu::Identifier Dnum::getType() const {
	return IDENT;
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

namespace {

static class : public torasu::DataPackableFactory {
	torasu::Identifier getType() const override {
		return IDENT;
	}

	torasu::UserLabel getLabel() const override {
		return {
			.name = "Number",
			.description = "Standard Number in TORASU"
		};
	}

	torasu::DataResource* create(const torasu::json* json) const override {
		return new Dnum(*json);
	}
} FACTORY_INSTANCE;

} // namespace

const torasu::DataPackableFactory* const Dnum::FACTORY = &FACTORY_INSTANCE;

} // namespace torasu::tstd
