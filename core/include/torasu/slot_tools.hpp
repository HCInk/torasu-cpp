#ifndef CORE_INCLUDE_TORASU_SLOT_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_SLOT_TOOLS_HPP_

#include <torasu/torasu.hpp>

namespace torasu::tools {

template<class T> class Slot {
protected:
	T* elem;
	bool owned = false;

public:
	inline Slot()
		: elem(nullptr) {}

	inline Slot(T* elem)
		: elem(elem) {}

	inline Slot(T* elem, bool owned)
		: elem(elem), owned(owned) {}

	inline T& operator*() {
		return *elem;
	}

	inline T& get() {
		return *elem;
	}
	inline T& isOwned() {
		return *elem;
	}

	virtual ~Slot() {}

};

template<class T> class ManagedSlot : public Slot<T> {
public:
	ManagedSlot(const Slot<T>& slot) : Slot<T>(slot) {}
	
	~ManagedSlot() {
		if (Slot<T>::owned) delete Slot<T>::elem;
	}
};

typedef Slot<torasu::Element> ElementSlot;
typedef Slot<torasu::Renderable> RenderableSlot;
typedef ManagedSlot<torasu::Element> ManagedElementSlot;
typedef ManagedSlot<torasu::Renderable> ManagedRenderableSlot;

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
