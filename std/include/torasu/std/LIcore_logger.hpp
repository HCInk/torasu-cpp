#ifndef STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
#define STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_

#include <torasu/torasu.hpp>

namespace torasu::tstd {

class LIcore_logger : public torasu::LogInterface {
    LogId log(LogEntry* entry, bool tag) override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
