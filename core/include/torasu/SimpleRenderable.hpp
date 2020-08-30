#ifndef CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
#define CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_

#include <string>
#include <map>
#include <iostream>

#include <torasu/torasu.hpp>

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

	// Implement when accepting elements
	std::map<std::string, Element*> getElements() override;
	void setElement(std::string key, Element* elem) override;

	// Auto-managed, overwrite to get more granular control over mass-setting of element linkage and data
	void setData(DataResource* data,
				 std::map<std::string, Element*> elements) override;
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
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) = 0;

public:
	virtual ~IndividualizedSegnentRenderable();
	RenderResult* render(RenderInstruction* ri) override;
};

/**
 * @brief  Renderable without RenderablePropierties
 */
class NoPropRenderable : public virtual Renderable {
protected:
	NoPropRenderable();
public:
	virtual ~NoPropRenderable();
	virtual RenderableProperties* getProperties();
};

/**
 * Class that combines the NamedIdentElement, SimpleDataElement and IndividualizedSegnentRenderable
 *
 * @brief  Collection of tools to simplify the implementation of Renderables with a low complexity
 */
class SimpleRenderable : public NamedIdentElement,
	public SimpleDataElement,
	public IndividualizedSegnentRenderable,
	public NoPropRenderable {

protected:
	explicit SimpleRenderable(std::string typeIdent, bool acceptData = false, bool acceptElements = false);

public:
	virtual ~SimpleRenderable();

};

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
