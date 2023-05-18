#ifndef STD_INCLUDE_TORASU_STD_RSTRING_MAP_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_MAP_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring.hpp>
#include <torasu/std/Rstring.hpp>

namespace torasu::tstd {

class Rstring_map : public torasu::tools::SimpleRenderable {
public:
	struct MapPair {
		std::string key;
		torasu::tstd::StringSlot slot;
	};

private:
	std::map<std::string, torasu::tools::ManagedSlot<torasu::tstd::StringSlot>> map;

public:
	explicit Rstring_map(std::initializer_list<MapPair> mapping);
	~Rstring_map();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_MAP_HPP_
