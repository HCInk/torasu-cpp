#ifndef CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
#define CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>

#define TORASU_PROPERTY_PREFIX "P#"
#define TORASU_PROPERTY_PREFIX_LEN 2
#define TORASU_PROPERTY(propName) TORASU_PROPERTY_PREFIX propName

namespace torasu {

typedef std::map<std::string, DataResource*> RenderableProperties;

inline bool isPipelineKeyPropertyKey(const std::string& pipelineKey) {
	return pipelineKey.length() > TORASU_PROPERTY_PREFIX_LEN &&
		   pipelineKey.substr(0, TORASU_PROPERTY_PREFIX_LEN).find(TORASU_PROPERTY_PREFIX, 0) == 0;
}

} // namespace torasu

#endif // CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
