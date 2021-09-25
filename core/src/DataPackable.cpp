/*
 * DataPackable.cpp
 *
 *  Created on: Mar 8, 2020
 */

#include "../include/torasu/DataPackable.hpp"

#include <utility>
#include <string>

#include <torasu/json.hpp>

#include "../include/torasu/torasu.hpp"

namespace torasu {

using namespace std;
using json = torasu::json;

/*
 * DP General
*/

DataPackable::DataPackable(const DataPackable* original, bool loaded) : loaded(loaded) {
	if (original->serializedJson.has_value())
		serializedJson = original->serializedJson.value();

	if (original->parsedJson.has_value())
		serializedJson = original->parsedJson.value();
}

DataPackable::DataPackable(bool loaded) : loaded(loaded) {}

DataPackable::DataPackable(std::string initialSerializedJson) {
	serializedJson = initialSerializedJson;
}

DataPackable::DataPackable(torasu::json initialJson) {
	parsedJson = initialJson;
}

DataPackable::~DataPackable() {}

void DataPackable::parse() {
	if (serializedJson.has_value()) {
		parsedJson = json::parse(serializedJson.value());
	} else {
		parsedJson = json::object();
	}
}

DataDump* DataPackable::dumpResource() {

	if (!serializedJson.has_value()) {
		serializedJson = getJson().dump();
	}

	const char* serJson_cStr = serializedJson.value().c_str();

	DDDataPointer pointer;
	pointer.s = serJson_cStr;

	return new DataDump(pointer, strlen(serJson_cStr), nullptr, true);
}

/*
 * DP Universal
*/


inline void DPUniversal::init() {
	json identJson = getJson()["ident"];
	if (identJson.is_string()) {
		ident = identJson;
	} else {
		ident = string();
	}
}

DPUniversal::DPUniversal(string jsonStripped) : DataPackable(jsonStripped) {
	init();
}
DPUniversal::DPUniversal(json jsonParsed) : DataPackable(jsonParsed) {
	init();
}

Identifier DPUniversal::getType() const {
	return ident.c_str();
}

void DPUniversal::load() {
	throw logic_error("load() of DPUniversal should never be called,"
					  " since it practically doesnt have a 'loaded' state");
}

json DPUniversal::makeJson() {
	throw logic_error("makeJson() of DPUniversal should never be called,"
					  " since it practically doesnt have a 'loaded' state");
}

DPUniversal* DPUniversal::clone() const {
	return new DPUniversal(*this);
}

} /* namespace torasu */
