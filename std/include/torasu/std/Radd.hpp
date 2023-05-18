#ifndef STD_INCLUDE_TORASU_STD_RADD_HPP_
#define STD_INCLUDE_TORASU_STD_RADD_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Radd : public tools::SimpleRenderable {
private:
	tools::ManagedSlot<NumSlot> a;
	tools::ManagedSlot<NumSlot> b;

public:
	Radd(NumSlot a, NumSlot b);
	~Radd();
	Identifier getType() override;

	RenderResult* render(RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RADD_HPP_
