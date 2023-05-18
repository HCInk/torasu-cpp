#ifndef STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_
#define STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rfloor_mod : public torasu::tools::SimpleRenderable {
private:
	tools::ManagedSlot<NumSlot> valRnd;
	tools::ManagedSlot<NumSlot> facRnd;

public:
	Rfloor_mod(NumSlot val, NumSlot fac);
	~Rfloor_mod();
	Identifier getType() override;

	RenderResult* render(RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;


};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RFLOOR_MOD_HPP_
