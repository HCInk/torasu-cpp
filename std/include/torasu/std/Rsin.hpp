#ifndef STD_INCLUDE_TORASU_STD_RSIN_HPP_
#define STD_INCLUDE_TORASU_STD_RSIN_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rsin : public tools::SimpleRenderable {
private:
	tools::ManagedSlot<NumSlot> valRnd;

public:
	explicit Rsin(NumSlot val);
	~Rsin();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSIN_HPP_
