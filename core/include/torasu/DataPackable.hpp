/*
 * DataPackable.h
 *
 *  Created on: Mar 8, 2020
 */

#ifndef DATAPACKABLE_H_
#define DATAPACKABLE_H_

#include <utility>
#include <string>

#include <nlohmann/json.hpp>

#include "torasu.hpp"


namespace torasu {

class DataPackable : public DataResource {
private:
	std::optional<std::string> serializedJson;
	std::optional<nlohmann::json> parsedJson;
	bool loaded = false;
	
	void parse();
protected:

	void inline ensureLoaded() {
		if (!loaded) {
			load();
			loaded = true;
		}
	}

	virtual void load() = 0;
	virtual nlohmann::json makeJson() = 0;

public:
	DataPackable();
	explicit DataPackable(std::string initialSerializedJson);
	explicit DataPackable(nlohmann::json initialJson);

	virtual ~DataPackable();

	DataDump getData();

	std::string inline getSerializedJson() {
		if (!serializedJson.has_value()) {
			serializedJson = getJson().dump();
		}
		return serializedJson.value();
	}

	nlohmann::json inline getJson() {
		if (!parsedJson.has_value()) {
			if (loaded) {
				parsedJson = makeJson();
				if (parsedJson.value().is_object()) {
					parsedJson.value()["ident"] = getIdent();
				}
			} else {
				parse();
			}
		}
		return parsedJson.value();
	}

};


class DPUniversal : public DataPackable {

private:
	std::optional<std::string> ident;

public:
	DPUniversal(std::string jsonStripped);
	DPUniversal(nlohmann::json jsonParsed);
	
	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();

};

} /* namespace torasu */

#endif /* DATAPACKABLE_H_ */
