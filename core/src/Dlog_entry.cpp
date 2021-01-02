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
	} else if (lvlStr == "DATA") {
		return torasu::LogLevel::DATA;
	} else {
		return torasu::LogLevel::UNKNOWN;
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
	case torasu::LogLevel::DATA:
		return "DATA";
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

	std::string msg;
	auto msgj = json["msg"];
	if (msgj.is_string()) {
		msg = msgj;
	} else {
		msg = "";
	}

	LogLevel lvl;
	auto lvlj = json["lvl"];
	if (msgj.is_string()) {
		lvl = lvlFromStr(lvlj);
	} else {
		lvl = torasu::LogLevel::UNKNOWN;
	}

	entry = new LogEntry(lvl, msg);

}

torasu::json Dlog_entry::makeJson() {
	return {
		{"msg", entry->message},
		{"lvl", lvlToStr(entry->level)},
	};
}

Dlog_entry* Dlog_entry::clone() {
	ensureLoaded();
	return new Dlog_entry(*entry);
}

} // namespace torasu
