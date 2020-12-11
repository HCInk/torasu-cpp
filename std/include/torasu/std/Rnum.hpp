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
	std::string pipeline = std::string(TORASU_STD_PL_NUM);

	DataResource* valdr;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	explicit Rnum(double val);
	explicit Rnum(Dnum val);
	virtual ~Rnum();

	virtual DataResource* getData();
	virtual void setData(DataResource* data);
};

class NumSlot : public torasu::tools::RenderableSlot {
public:
	inline NumSlot() {}

	inline NumSlot(Renderable* rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	inline NumSlot(torasu::tools::RenderableSlot rnd)
		: torasu::tools::RenderableSlot(rnd) {}

	inline NumSlot(double num)
		: torasu::tools::RenderableSlot(new Rnum(num), true) {}

	inline NumSlot(Dnum num)
		: torasu::tools::RenderableSlot(new Rnum(num), true) {}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
