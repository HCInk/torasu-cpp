#ifndef STD_INCLUDE_TORASU_STD_RSIN_HPP_
#define STD_INCLUDE_TORASU_STD_RSIN_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rsin : public tools::SimpleRenderable {
private:
	tools::ManagedSlot<NumSlot> valRnd;

protected:
	ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	Rsin(NumSlot val);
	~Rsin();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSIN_HPP_
