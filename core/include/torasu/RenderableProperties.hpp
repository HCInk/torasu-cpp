#ifndef CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
#define CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_

#include <set>
#include <map>

#include <torasu/torasu.hpp>

#define TORASU_PROPERTY_PREFIX "P#"
#define TORASU_PROPERTY_PREFIX_LEN 2

namespace torasu {

typedef std::map<std::string, DataResource*> RenderableProperties;

class PropertyInstruction {
private:
	std::set<std::string>* rProps;
	RenderContext* rctx;
	ExecutionInterface* ei;

public:
	inline PropertyInstruction(std::set<std::string>* rProps, RenderContext* rctx, ExecutionInterface* ei)
		: rProps(rProps), rctx(rctx), ei(ei) {}

	~PropertyInstruction() {}

	inline RenderContext* const getRenderContext() const {
		return rctx;
	}

	inline std::set<std::string>* getRequestedProperties() {
		return rProps;
	}

	/**
	 * @brief  Checks if a property is requested and then removes it from the list of requested properties
	 * @param  key: The property-key to be checked
	 * @retval true: The key was found and removed from the requests; false: the key was not found in the requests
	 */
	inline bool checkPopProperty(std::string key) {
		auto found = rProps->find(key);
		if (found != rProps->end()) {
			rProps->erase(found);
			return true;
		} else {
			return false;
		}
	}

	inline ExecutionInterface* const getExecutionInterface() const {
		return ei;
	}
};

} // namespace torasu


#endif // CORE_INCLUDE_TORASU_RENDERABLEPROPERTIES_HPP_
