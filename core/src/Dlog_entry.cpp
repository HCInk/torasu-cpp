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
	} else if (typeStr == "GUNREF") {
		return torasu::LogType::LT_GROUP_UNREF;
	} else if (typeStr == "GPERS") {
		return torasu::LogType::LT_GROUP_PERSIST;
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
	case torasu::LogType::LT_GROUP_UNREF:
		return "GUNREF";
	case torasu::LogType::LT_GROUP_PERSIST:
		return "GPERS";
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

	switch (type) {
	case LT_MESSAGE: {
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

			entry = new LogMessage(lvl, txt);
			break;
		}
	case LT_GROUP_START: {
			std::string name;
			auto namej = json["name"];
			if (namej.is_string()) {
				name = namej;
			} else {
				name = "";
			}

			entry = new LogGroupStart(name);
			break;
		}
	case LT_DATA: {
			std::string text;
			auto textj = json["text"];
			if (textj.is_string()) {
				text = textj;
			} else {
				text = "";
			}

			// TODO Data parsing needs "Nested-DataResource-Import" feature
			if (json.contains("data")) {
				auto dataj = json["data"];
				text += " [Data will currently not be parsed - RAW: " + dataj.dump() + "]";
			}

			entry = new LogData(text, nullptr);
			break;
		}
	default:
		entry = new LogEntry(type);
		break;
	}

}

torasu::json Dlog_entry::makeJson() {
	torasu::json json;

	json["t"] = typeToStr(entry->type);

	switch (entry->type) {
	case LT_MESSAGE: {
			LogMessage* msg = static_cast<LogMessage*>(entry);
			json["txt"] = msg->text;
			json["lvl"] = lvlToStr(msg->level);
			break;
		}
	case LT_GROUP_START: {
			LogGroupStart* start = static_cast<LogGroupStart*>(entry);
			json["name"] = start->name;
			break;
		}
	case LT_DATA: {
			LogData* dataLog = static_cast<LogData*>(entry);
			json["name"] = dataLog->text;
			// TODO Data serializing needs "Nested-DataResource-Export" feature
			if (dataLog->data != nullptr) {
				json["data"] = "[data currently not able to be serialized]";
			}
			break;
		}
	default:
		break;
	}

	return json;
}

Dlog_entry* Dlog_entry::clone() {
	ensureLoaded();
	return new Dlog_entry(*entry);
}

} // namespace torasu
