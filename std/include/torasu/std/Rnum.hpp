#ifndef STD_INCLUDE_TORASU_STD_RNUM_HPP_
#define STD_INCLUDE_TORASU_STD_RNUM_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>

namespace torasu::tstd {

class Rnum : public tools::SimpleRenderable {
private:
	DataResource* valdr;

public:
	explicit Rnum(double val);
	explicit Rnum(Dnum val);
	~Rnum();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	DataResource* getData() override;
	void setData(DataResource* data) override;

	static const torasu::ElementFactory* const FACTORY;
};

class NumSlot : public torasu::tools::RenderableSlot {
public:
	inline NumSlot() {}

	/* implicit */ inline NumSlot(Renderable* rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	/* implicit */ inline NumSlot(torasu::tools::RenderableSlot rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	/* implicit */ inline NumSlot(double num)
		: torasu::tools::RenderableSlot(new Rnum(num), true) {}

	/* implicit */ inline NumSlot(Dnum num)
		: torasu::tools::RenderableSlot(new Rnum(num), true) {}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
