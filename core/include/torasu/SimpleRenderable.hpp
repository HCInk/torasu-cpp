#ifndef CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
#define CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/RenderableProperties.hpp>

namespace torasu::tools {

/**
 * Element that lets you declare a fixed type-ident
 * that will constantly be provided over Element#getType()
 *
 * @brief  Element with a fixed type-ident
 */
class NamedIdentElement : public virtual Element {

private:
	std::string typeIdent;

protected:
	explicit NamedIdentElement(std::string typeIdent);

public:
	virtual ~NamedIdentElement();

	// Auto-managed
	std::string getType() override;
};

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
	void setElement(std::string key, Element* elem) override;

	// Auto-managed, overwrite to get more granular control over mass-setting of element linkage and data
	void setData(DataResource* data,
				 torasu::ElementMap elements) override;
};

/**
 * Renderable which calls for every segment in the ResultSettings renderSegment(),
 * which then has to process the given segement and return the matching ResultSegment.
 * Those ResultSegment will then be packed into the RenderResult together with others automatically by this class.
 *
 * @brief  Individualizes multiple segments in the ResultSettings into one call per segment
 */
class IndividualizedSegnentRenderable : public virtual Renderable {
protected:
	IndividualizedSegnentRenderable();
	// Implement to handle the processing of render-segments
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) = 0;

public:
	virtual ~IndividualizedSegnentRenderable();
	RenderResult* render(RenderInstruction* ri) override;
};

/**
 * @brief  Element which overrides Ready-functions whith dummies, so they dont have to be explitily implemented
 */
class ReadylessElement : public virtual Element {
protected:
	ReadylessElement();

public:
	virtual ~ReadylessElement();

	ReadyObjects* requestReady(const ReadyRequest& ri) override;
	ElementReadyResult* ready(const ReadyInstruction& ri) override;
	void unready(const UnreadyInstruction& uri) override;
};

/**
 * Class that combines the NamedIdentElement, SimpleDataElement and IndividualizedSegnentRenderable
 *
 * @brief  Collection of tools to simplify the implementation of Renderables with a low complexity
 */
class SimpleRenderable : public NamedIdentElement,
	public SimpleDataElement,
	public IndividualizedSegnentRenderable,
	public ReadylessElement {

protected:
	explicit SimpleRenderable(std::string typeIdent, bool acceptData, bool acceptElements);

public:
	virtual ~SimpleRenderable();

};

/**
 * @brief  Shorthand to make setting renderable-slots easier
 * @param  slotKey: Slot key of the slot to be put into
 * @param  rndSlot: Slot address the Renderable should be put into
 * @param  supportNull: If the slot can be set to null (if not an exception will be thrown, when null-assignments happen)
 * @param  givenKey: The key that is getting set (and which is getting set)
 * @param  givenElement: The given element to be set
 * @param  ownsCurrent: Bool wether the element is owned: On a match true will delete the current element and set the value to false
 * @retval If the element has been set into the slot
 */
inline bool trySetRenderableSlot(const char* slotKey, torasu::Renderable** rndSlot, bool supportNull, const std::string& givenKey, torasu::Element* givenElement, bool* ownsCurrent = nullptr) {
	if (givenKey.compare(slotKey) == 0) {
		if (givenElement == nullptr) {
			if (supportNull) {
				if (ownsCurrent != nullptr || *ownsCurrent) {
					*ownsCurrent = false;
					delete rndSlot;
				}
				*rndSlot = nullptr;
				return true;
			} else {
				throw std::invalid_argument(std::string("Element slot \"") + slotKey + std::string("\" may not be empty!"));
			}
		}
		if (torasu::Renderable* rnd = dynamic_cast<torasu::Renderable*>(givenElement)) {
			if (ownsCurrent != nullptr || *ownsCurrent) {
				*ownsCurrent = false;
				delete rndSlot;
			}
			*rndSlot = rnd;
			return true;
		} else {
			throw std::invalid_argument(std::string("Element slot \"") + slotKey + std::string("\" only accepts Renderables!"));
		}
	}
	return false;
}

/**
 * @brief  Create exception that an element slot doesnt exist
 * @param  key: The key of the element slot that was requested
 * @retval The generated exception
 */
inline std::exception makeExceptSlotDoesntExist(const std::string& key) {
	return std::invalid_argument(std::string("The element slot \"") + key + "\" does not exist!");
}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
