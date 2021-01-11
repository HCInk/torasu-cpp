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

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	explicit Rstring(std::string str);
	~Rstring();

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

class StringSlot : public torasu::tools::RenderableSlot {
public:
	inline StringSlot() {}

	/* implicit */ inline StringSlot(Renderable* rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	/* implicit */ inline StringSlot(torasu::tools::RenderableSlot rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	/* implicit */ inline StringSlot(const char* str)
		: torasu::tools::RenderableSlot(new Rstring(str), true) {}

	/* implicit */ inline StringSlot(std::string str)
		: torasu::tools::RenderableSlot(new Rstring(str), true) {}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_HPP_
