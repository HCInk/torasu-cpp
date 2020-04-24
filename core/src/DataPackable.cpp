/*
 * DataPackable.cpp
 *
 *  Created on: Mar 8, 2020
 */

#include "../include/torasu/DataPackable.hpp"

#include <iostream>
#include <utility>
#include <string>
#include <nlohmann/json.hpp>

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
	// TODO delete c-string in currentJson
	//free(currentJson); // FIXME is this correct?
}

void DataPackable::parse() {
	if (serializedJson.has_value()) {
		parsedJson = json::parse(serializedJson.value());
	} else {
		parsedJson = json::object();
	}
}

/*void DP_FreeDump(DataDump* dump) {
	if (dump->getFormat() == DDDataPointerType::DDDataPointerType_JSON_CSTR) {
		cout << "DALLOC" << endl;
		//delete[] (const char*) dump->getData().s;
	}
}*/

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

DPUniversal::DPUniversal(std::string jsonStripped) : DataPackable(jsonStripped) {}
DPUniversal::DPUniversal(nlohmann::json jsonParsed) : DataPackable(jsonParsed) {}

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

nlohmann::json DPUniversal::makeJson() {
	throw logic_error("makeJson() of DPUniversal should never be called,"
		" since it practically doesnt have a 'loaded' state");
}

} /* namespace torasu */
