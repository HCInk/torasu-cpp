#ifndef CORE_INCLUDE_TORASU_SLOT_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_SLOT_TOOLS_HPP_

#include <torasu/torasu.hpp>

namespace torasu::tools {

template<class T> class ManagedSlot : public T {
public:
	ManagedSlot() {}
	/* implicit */ ManagedSlot(const T& slot) : T(slot) {}
	ManagedSlot(const ManagedSlot<T>& slot) = delete;

	inline ManagedSlot<T>& operator=(const T& b) {
		if (T::owned) delete T::elem;
		T::elem = b.get();
		T::owned = b.isOwned();
		return *this;
	}

	inline const ElementSlot asElementSlot() const {
		return ElementSlot(T::elem, T::owned);
	}

	~ManagedSlot() {
		if (T::owned) delete T::elem;
	}
};

typedef ManagedSlot<ElementSlot> ManagedElementSlot;
typedef ManagedSlot<RenderableSlot> ManagedRenderableSlot;

/**
 * @brief  Creates an ElementSlot which should be freed automatically
 * @note   Putting an Element which is freed somewhere can lead to undefined behaviour
 * @param  elem: The Element to be put into the slot
 * @retval The ElementSlot
 */
inline ElementSlot inlineElement(torasu::Element* elem) {
	return ElementSlot(elem, true);
}

/**
 * @brief  Creates a RenderableSlot which should be freed automatically
 * @note   Putting an Renderable which is freed somewhere can lead to undefined behaviour
 * @param  rnd: The Renderable to be put into the slot
 * @retval The RenderableSlot
 */
inline RenderableSlot inlineRenderable(torasu::Renderable* rnd) {
	return RenderableSlot(rnd, true);
}

} // namespace torasu::tools


#endif // CORE_INCLUDE_TORASU_SLOT_TOOLS_HPP_
