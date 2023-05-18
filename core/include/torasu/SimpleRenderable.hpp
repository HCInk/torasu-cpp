#ifndef CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
#define CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_

#include <string>
#include <vector>

#include <torasu/torasu.hpp>
#include <torasu/RenderableProperties.hpp>
#include <torasu/slot_tools.hpp>

namespace torasu::tools {

/**
 * Element that lets you remaps all methods of an Element
 * for setting Data / Elements to setData() and setElement().
 * It also provides sensible defaults/fallbacks for those.
 *
 * @brief  Element with simplified data-/connection-management
 */
class SimpleDataElement : public virtual Element {

private:
	bool acceptData, acceptElements;

protected:
	SimpleDataElement(bool acceptData, bool acceptElements);

public:
	virtual ~SimpleDataElement();

	// Overwrite when accepting data
	DataResource* getData() override;
	void setData(DataResource* data) override;

	// Overwrite when accepting elements
	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const ElementSlot* elem) override;

	// Auto-managed, overwrite to get more granular control over mass-setting of element linkage and data
	void setData(DataResource* data,
				 torasu::ElementMap elements) override;
};

class NoneReadyState : public ReadyState {
private:
	std::vector<Identifier>* operations;
	RenderContextMask* rctxm;
public:
	explicit NoneReadyState(const std::vector<Identifier>& operations);
	NoneReadyState(const NoneReadyState& orig);
	~NoneReadyState();
	const std::vector<Identifier>* getOperations() const override;
	const RenderContextMask* getContextMask() const override;
	size_t size() const override;
	NoneReadyState* clone() const override;
};

/**
 * @brief  Element which overrides Ready-functions whith dummies, so they dont have to be explitily implemented
 */
class ReadylessElement : public virtual Element {
protected:
	ReadylessElement();

public:
	virtual ~ReadylessElement();

	void ready(ReadyInstruction* ri) override;
};

/**
 * Class that combines the NamedIdentElement, SimpleDataElement, ReadylessElement
 *
 * @brief  Collection of tools to simplify the implementation of Renderables with a low complexity
 */
class SimpleRenderable : public SimpleDataElement,
	public ReadylessElement,
	public Renderable {

protected:
	explicit SimpleRenderable(bool acceptData, bool acceptElements);

public:
	virtual ~SimpleRenderable();

};

template<class T> inline const torasu::ElementSlot trySetRenderableSlot(torasu::tools::ManagedSlot<T>* slot, const torasu::ElementSlot* given) {
	if (given == nullptr) {
		*slot = RenderableSlot(nullptr, false);
	} else if (torasu::Renderable* rnd = dynamic_cast<torasu::Renderable*>(given->get())) {
		*slot = RenderableSlot(rnd, given->isOwned());
	}
	return slot->asElementSlot();
}

template<class T, class K> inline const torasu::OptElementSlot trySetRenderableSlot(std::map<K, torasu::tools::ManagedSlot<T>>* slotMap, const K& key, const torasu::ElementSlot* given) {
	if (given == nullptr) {
		slotMap->erase(key);
		return nullptr;
	} else if (torasu::Renderable* rnd = dynamic_cast<torasu::Renderable*>(given->get())) {
		torasu::tools::ManagedSlot<T>& slot = (*slotMap)[key];
		slot = RenderableSlot(rnd, given->isOwned());
		return slot.asElementSlot();
	} else {
		const auto found = slotMap->find(key);
		return found != slotMap->end() ? found->second.asElementSlot() : nullptr;
	}
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
