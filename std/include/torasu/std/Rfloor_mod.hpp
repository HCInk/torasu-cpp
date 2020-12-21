#ifndef STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_
#define STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rfloor_mod : public torasu::tools::SimpleRenderable {
private:
	tools::ManagedSlot<NumSlot> valRnd;
	tools::ManagedSlot<NumSlot> facRnd;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rfloor_mod(NumSlot val, NumSlot fac);
	~Rfloor_mod();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;


};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_
