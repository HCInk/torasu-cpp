#include "../include/torasu/std/LIcore_logger.hpp"

#include <memory>
#include <iostream>

namespace {

const char* getLvLName(torasu::LogLevel lvl) {
    switch (lvl) {
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

} // namespace


namespace torasu::tstd {

LogId LIcore_logger::log(LogEntry* entryIn, bool tag) {
    std::unique_ptr<LogEntry> entry(entryIn);

    std::cout << getLvLName(entry->level) << "  " << entry->message << std::endl;

    return 0;
}

} // namespace torasu::tstd
