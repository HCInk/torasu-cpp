// TORASU-header, which wrapps the nlohmann-json library installed with torasu

#ifndef LIB_INCLUDE_TORASU_JSON_HPP_
#define LIB_INCLUDE_TORASU_JSON_HPP_

#include "lib/nlohmann/json.hpp"

namespace torasu {
// nlohmann-json datatype brought with torasu
typedef nlohmann::json json;
} // namespace torasu

#endif // LIB_INCLUDE_TORASU_JSON_HPP_
