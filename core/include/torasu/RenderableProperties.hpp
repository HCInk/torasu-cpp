#ifndef CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
#define CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_

#include <cstring>
#include <string>
#include <map>

#include <torasu/torasu.hpp>

#define TORASU_PROPERTY_PREFIX "P#"
#define TORASU_PROPERTY_PREFIX_LEN 2
#define TORASU_PROPERTY(propName) TORASU_PROPERTY_PREFIX propName

namespace torasu {

typedef std::map<std::string, DataResourceHolder> RenderableProperties;

inline bool isPipelineKeyPropertyKey(torasu::Identifier pipelineKey) {
	for (size_t i = 0; i < TORASU_PROPERTY_PREFIX_LEN; i++) {
		if (pipelineKey.str[i] != TORASU_PROPERTY_PREFIX[i]) return false;
	}
	return true;
}

} // namespace torasu

#endif // CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
