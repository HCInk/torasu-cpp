#include "../include/torasu/std/Dstring_map.hpp"

namespace torasu::tstd {


Dstring_map::Dstring_map(std::string jsonStripped) : DataPackable(jsonStripped) {}
Dstring_map::Dstring_map(torasu::json jsonParsed) : DataPackable(jsonParsed) {}

Dstring_map::Dstring_map() {}
Dstring_map::~Dstring_map() {}

const std::map<std::string, std::string>& Dstring_map::getMap() const {
	return map;
}

const std::string* Dstring_map::get(const std::string& key) const {
	auto found = map.find(key);
	if (found != map.end()) {
		return &found->second;
	} else {
		return nullptr;
	}
}

void Dstring_map::set(const std::string& key, const std::string& value) {
	// TODO Add way to mark update of DP-json
	map[key] = value;
}

void Dstring_map::erase(const std::string& key) {
	// TODO Add way to mark update of DP-json
	map.erase(key);
}

void Dstring_map::load() {
	torasu::json json = getJson();

	for (auto& entry : json.items()) {
		if (entry.value().is_string()) {
			map[entry.key()] = entry.value();
		}
	}
}

torasu::json Dstring_map::makeJson() {
	torasu::json json;

	for (auto& entry : map) {
		json[entry.first] = entry.second;
	}

	return json;
}

std::string Dstring_map::getIdent() {
	return "STD::STR_MAP";
}

Dstring_map* Dstring_map::clone() {
	return new Dstring_map(*this);
}

} // namespace torasu::tstd