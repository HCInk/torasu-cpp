#ifndef STD_INCLUDE_TORASU_STD_RSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring.hpp>

namespace torasu::tstd {

class Rstring : public torasu::tools::SimpleRenderable {
private:
	Dstring* str;

public:
	explicit Rstring(std::string str);
	~Rstring();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

	static const torasu::ElementFactory* const FACTORY;
};

class StringSlot : public torasu::RenderableSlot {
public:
	inline StringSlot() {}

	/* implicit */ inline StringSlot(Renderable* rnd)
		: torasu::RenderableSlot(rnd) {}

	/* implicit */ inline StringSlot(torasu::RenderableSlot rnd)
		: torasu::RenderableSlot(rnd) {}

	/* implicit */ inline StringSlot(const char* str)
		: torasu::RenderableSlot(new Rstring(str), true) {}

	/* implicit */ inline StringSlot(std::string str)
		: torasu::RenderableSlot(new Rstring(str), true) {}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_HPP_
