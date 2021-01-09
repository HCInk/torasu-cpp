#include "../include/torasu/std/LIcore_logger.hpp"

#include <memory>
#include <iostream>

namespace {

const char* getLvLName(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return "TRACE";
	case torasu::LogLevel::DEBUG:
		return "DEBUG";
	case torasu::LogLevel::INFO:
		return "INFO ";
	case torasu::LogLevel::WARN:
		return "WARN ";
	case torasu::LogLevel::ERROR:
		return "ERROR";
	case torasu::LogLevel::SERVERE_ERROR:
		return "S-ERR";
	default:
		return "UNKWN";
	}
}

static const char* ANSI_RESET = "\33[0m";
static const char* ANSI_HIGHLIGTED_RED = "\33[97m\33[101m";
static const char* ANSI_RED = "\33[91m";
static const char* ANSI_YELLOW = "\33[93m";
static const char* ANSI_BLUE = "\33[94m";
static const char* ANSI_DARK_GREEN = "\33[32m";
static const char* ANSI_GRAY = "\33[90m";
static const char* ANSI_CYAN = "\33[36m";

const char* getLvlAnsi(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return ANSI_GRAY;
	case torasu::LogLevel::DEBUG:
		return ANSI_DARK_GREEN;
	case torasu::LogLevel::INFO:
		return ANSI_BLUE;
	case torasu::LogLevel::WARN:
		return ANSI_YELLOW;
	case torasu::LogLevel::ERROR:
		return ANSI_RED;
	case torasu::LogLevel::SERVERE_ERROR:
		return ANSI_HIGHLIGTED_RED;
	default:
		return "";
	}
}

static std::string groupStackToStr(std::vector<torasu::LogId> groupStack) {
	auto gsit = groupStack.rbegin();
	std::string str;
	while (gsit != groupStack.rend()) {
		str += "/" + std::to_string(*gsit);
		gsit++;
	}

	return str;
}

} // namespace


namespace torasu::tstd {

LIcore_logger::LIcore_logger() {}

LIcore_logger::LIcore_logger(bool useAnsi) : useAnsi(useAnsi) {}

LogId LIcore_logger::log(LogEntry* entryIn, bool tag) {
	std::unique_ptr<LogEntry> entry(entryIn);

	std::string message;

	switch (entry->type) {
	case LT_GROUP_START: {
		if (useAnsi) message += ANSI_GRAY;
		message += "====  " + groupStackToStr(entry->groupStack) + " * " + entry->text;
	} break;

	case LT_GROUP_END: {
	} break;
	
	default: {
		// Prefix

		if (entry->type == LT_MESSAGE) {
			if (useAnsi) message += getLvlAnsi(entry->level);
			message += getLvLName(entry->level);
		} else {
			if (useAnsi) message += ANSI_CYAN;
			message += "DATA ";
		}

		message += " ";

		// Group Display

		if (!entry->groupStack.empty()) {
			message += groupStackToStr(entry->groupStack) + " \t";
		}

		// Message

		if (entry->type == LT_MESSAGE) {
			message += entry->text;
		} else {
			message += "[DataPacket-" + std::to_string(entry->type) + "] TXT: \"" + entry->text + "\""; 
		}

		
	} break;
	}

	if (!message.empty()) {
		// Suffix

		if (useAnsi) {
			message += ANSI_RESET;
		}

		// Print

		std::cout << message << std::endl;
	}

	return 0;
}

torasu::LogId LIcore_logger::fetchSubId() {

	std::unique_lock lock(subIdCounterLock);
	auto subId = subIdCounter;
	subIdCounter++;
	return subId;

}

} // namespace torasu::tstd
