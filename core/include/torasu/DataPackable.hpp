/*
 * DataPackable.h
 *
 *  Created on: Mar 8, 2020
 */

#ifndef CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_
#define CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_

#include <utility>
#include <string>

#include <nlohmann/json.hpp>
#ifdef _WIN32

#include <optional>
#endif
#include "torasu.hpp"


namespace torasu {

class DataPackable : public DataResource {
private:
	std::optional<std::string> serializedJson;
	std::optional<nlohmann::json> parsedJson;
	bool loaded = false;

	 DLL_EXPORT void __cdecl parse();
protected:

	void inline setLoaded() {
		loaded = true;
	}

	void inline ensureLoaded() {
		if (!loaded) {
			load();
			loaded = true;
		}
	}

	virtual void load() = 0;
	virtual nlohmann::json makeJson() = 0;

public:
	DLL_EXPORT __cdecl DataPackable();
	DLL_EXPORT explicit __cdecl DataPackable(std::string initialSerializedJson);
	DLL_EXPORT explicit __cdecl DataPackable(nlohmann::json initialJson);

	virtual  ~DataPackable();

	DataDump* getData();

 DLL_EXPORT std::string inline __cdecl getSerializedJson() {
		if (!serializedJson.has_value()) {
			serializedJson = getJson().dump();
		}
		return serializedJson.value();
	}

	 DLL_EXPORT nlohmann::json inline __cdecl getJson() {
		if (!parsedJson.has_value()) {
			if (loaded) {
				parsedJson = makeJson();
				if (parsedJson.value().is_object()) {
					parsedJson.value()["ident"] = getIdent();
				}
			} else {
		    	this->parse();
			}
		}
		return parsedJson.value();
	}

};

class DLL_EXPORT DPUniversal : public DataPackable {

private:
	std::optional<std::string> ident;

public:
	explicit DPUniversal(std::string jsonStripped);
	explicit DPUniversal(nlohmann::json jsonParsed);

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();

};

} /* namespace torasu */

#endif // CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_
