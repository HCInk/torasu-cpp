#ifndef CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_

#include <string>

#include <torasu/torasu.hpp>

namespace torasu::tools {

inline void log_checked(torasu::LogInstruction li, torasu::LogLevel lvl, std::string msg) {
	if (li.level <= lvl) li.logger->log(lvl, msg);
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_
