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

class NumSlot : public torasu::RenderableSlot {
public:
	inline NumSlot() {}

	/* implicit */ inline NumSlot(Renderable* rnd)
		: torasu::RenderableSlot(rnd) {}

	/* implicit */ inline NumSlot(torasu::RenderableSlot rnd)
		: torasu::RenderableSlot(rnd) {}

	/* implicit */ inline NumSlot(double num)
		: torasu::RenderableSlot(new Rnum(num), true) {}

	/* implicit */ inline NumSlot(Dnum num)
		: torasu::RenderableSlot(new Rnum(num), true) {}

	static inline const std::optional<ElementSlot> trySetRenderableSlot(torasu::tools::ManagedSlot<NumSlot>* slot, const torasu::ElementSlot* given, double defaultValue) {
		if (given == nullptr) {
			*slot = defaultValue;
		} else if (torasu::Renderable* rnd = dynamic_cast<torasu::Renderable*>(given->get())) {
			*slot = RenderableSlot(rnd, given->isOwned());
		}

		return slot->asElementSlot();
	}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
