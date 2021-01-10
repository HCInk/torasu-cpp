#include "../include/torasu/Dlog_entry.hpp"

namespace {

torasu::LogLevel lvlFromStr(std::string lvlStr) {
	if (lvlStr == "TRC") {
		return torasu::LogLevel::TRACE;
	} else if (lvlStr == "DBG") {
		return torasu::LogLevel::DEBUG;
	} else if (lvlStr == "INFO") {
		return torasu::LogLevel::INFO;
	} else if (lvlStr == "WARN") {
		return torasu::LogLevel::WARN;
	} else if (lvlStr == "ERROR") {
		return torasu::LogLevel::ERROR;
	} else if (lvlStr == "SERVERR") {
		return torasu::LogLevel::SERVERE_ERROR;
	} else {
		return torasu::LogLevel::LEVEL_UNKNOWN;
	}
}

const char* lvlToStr(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return "TRC";
	case torasu::LogLevel::DEBUG:
		return "DBG";
	case torasu::LogLevel::INFO:
		return "INFO";
	case torasu::LogLevel::WARN:
		return "WARN";
	case torasu::LogLevel::ERROR:
		return "ERROR";
	case torasu::LogLevel::SERVERE_ERROR:
		return "SERVERR";
	default:
		return "UNK";
	}
}

torasu::LogType typeFromStr(std::string typeStr) {
	if (typeStr == "MSG") {
		return torasu::LogType::LT_MESSAGE;
	} else if (typeStr == "GSTART") {
		return torasu::LogType::LT_GROUP_START;
	} else if (typeStr == "GEND") {
		return torasu::LogType::LT_GROUP_END;
	} else {
		return torasu::LogType::LT_UNKNOWN;
	}
}

const char* typeToStr(torasu::LogType type) {
	switch (type) {
	case torasu::LogType::LT_MESSAGE:
		return "MSG";
	case torasu::LogType::LT_GROUP_START:
		return "GSTART";
	case torasu::LogType::LT_GROUP_END:
		return "GEND";
	default:
		return "UNK";
	}
}

} // namespace

namespace torasu {

Dlog_entry::Dlog_entry(const LogEntry& entry)
	: entry(new LogEntry(entry)) {}

Dlog_entry::Dlog_entry(torasu::json json)
	: DataPackable(json) {}

Dlog_entry::Dlog_entry(std::string jsonStr)
	: DataPackable(jsonStr) {}

Dlog_entry::~Dlog_entry() {
	if (entry) delete entry;
}

Dlog_entry* LogEntry::makePack() {
	return new Dlog_entry(*this);
}

LogEntry Dlog_entry::getEntry() {
	ensureLoaded();
	return *entry;
}

std::string Dlog_entry::getIdent() {
	return "T::DLOG_ENTRY";
}

void Dlog_entry::load() {
	json json = getJson();

	LogType type;
	auto typej = json["t"];
	if (typej.is_string()) {
		type = typeFromStr(typej);
	} else {
		type = LT_UNKNOWN;
	}

	std::string txt;
	auto txtj = json["txt"];
	if (txtj.is_string()) {
		txt = txtj;
	} else {
		txt = "";
	}

	LogLevel lvl;
	auto lvlj = json["lvl"];
	if (lvlj.is_string()) {
		lvl = lvlFromStr(lvlj);
	} else {
		lvl = torasu::LogLevel::LEVEL_UNKNOWN;
	}

	entry = new LogEntry(type, lvl, txt);

}

torasu::json Dlog_entry::makeJson() {
	torasu::json json;

	json["t"] = typeToStr(entry->type);

	if (!entry->text.empty())
		json["txt"] = entry->text;

	if (entry->type == torasu::LogType::LT_MESSAGE)
		json["lvl"] = lvlToStr(entry->level);

	return json;
}

Dlog_entry* Dlog_entry::clone() {
	ensureLoaded();
	return new Dlog_entry(*entry);
}

} // namespace torasu
