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
	case torasu::LogLevel::DATA:
		return "DATA ";
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
static const char* ANSI_BRIGHT_GREEN = "\33[32m";

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
	case torasu::LogLevel::DATA:
		return ANSI_BRIGHT_GREEN;
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

	if (!entry->groupStack.empty()) {
		message += groupStackToStr(entry->groupStack) + "\t";
	}

	message += entry->message;

	if (useAnsi) {
		std::cout
				<< getLvlAnsi(entry->level)
				<< getLvLName(entry->level)
				<< "  "
				<< message
				<< ANSI_RESET
				<< std::endl;
	} else {
		std::cout
				<< getLvLName(entry->level)
				<< "  "
				<< message
				<< std::endl;
	}

	return 0;
}

} // namespace torasu::tstd
