/*
 * DataPackable.h
 *
 *  Created on: Mar 8, 2020
 */

#ifndef CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_
#define CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_

#include <utility>
#include <optional>
#include <string>

#include <torasu/json.hpp>

#include "torasu.hpp"


namespace torasu {

class DataPackable : public virtual DataResource {
private:
	std::optional<std::string> serializedJson;
	std::optional<torasu::json> parsedJson;
	bool loaded = false;

	void parse();
protected:

	void inline setUpdate() {
		serializedJson.reset();
		parsedJson.reset();
	}

	void inline setLoaded() {
		loaded = true;
	}

	bool inline isLoaded() const {
		return loaded;
	}

	void inline ensureLoaded() {
		if (!loaded) {
			load();
			loaded = true;
		}
	}

	virtual void load() = 0;
	virtual torasu::json makeJson() = 0;

public:
	explicit DataPackable(const DataPackable* original, bool loaded);
	explicit DataPackable(bool loaded = true);
	explicit DataPackable(std::string initialSerializedJson);
	explicit DataPackable(torasu::json initialJson);

	virtual ~DataPackable();

	DataDump* dumpResource() override;

	std::string inline getSerializedJson() {
		if (!serializedJson.has_value()) {
			serializedJson = getJson().dump();
		}
		return serializedJson.value();
	}

	torasu::json inline getJson() {
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
	std::string ident;
	void init();

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	explicit DPUniversal(std::string jsonStripped);
	explicit DPUniversal(torasu::json jsonParsed);

	std::string getIdent() const override;
	DPUniversal* clone() const override;
};

} /* namespace torasu */

#endif // CORE_INCLUDE_TORASU_DATAPACKABLE_HPP_
