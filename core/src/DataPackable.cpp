/*
 * DataPackable.cpp
 *
 *  Created on: Mar 8, 2020
 */

#include "../include/torasu/DataPackable.hpp"

#include <nlohmann/json.hpp>

#include <iostream>
#include <utility>
#include <string>

#include "../include/torasu/torasu.hpp"

namespace torasu {

using namespace std;
using json = nlohmann::json;

/*
 * DP General
*/

DataPackable::DataPackable() {}

DataPackable::DataPackable(std::string initialSerializedJson) {
	serializedJson = initialSerializedJson;
}

DataPackable::DataPackable(nlohmann::json initialJson) {
	parsedJson = initialJson;
}

DataPackable::~DataPackable() {
}

void DataPackable::parse() {
	if (serializedJson.has_value()) {
		parsedJson = json::parse(serializedJson.value());
	} else {
		parsedJson = json::object();
	}
}

DataDump DataPackable::getData() {

	if (!serializedJson.has_value()) {
		serializedJson = getJson().dump();
	}
	
	const char* serJson_cStr = serializedJson.value().c_str();

	DDDataPointer pointer;
	pointer.s = serJson_cStr;

	return DataDump(pointer, strlen(serJson_cStr), 
		DDDataPointerType::DDDataPointerType_JSON_CSTR, NULL);
}

/*
 * DP Universal
*/

DPUniversal::DPUniversal(string jsonStripped) : DataPackable(jsonStripped) {}
DPUniversal::DPUniversal(json jsonParsed) : DataPackable(jsonParsed) {}

std::string DPUniversal::getIdent() {
	if (!ident.has_value()) {
		json identJson = getJson()["ident"];
		if (identJson.is_string()) {
			ident = identJson;
		} else {
			ident = string();
		}
	}
	return ident.value();
}

void DPUniversal::load() {
	throw logic_error("load() of DPUniversal should never be called,"
		" since it practically doesnt have a 'loaded' state");
}

json DPUniversal::makeJson() {
	throw logic_error("makeJson() of DPUniversal should never be called,"
		" since it practically doesnt have a 'loaded' state");
}

} /* namespace torasu */
