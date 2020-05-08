#ifndef CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
#define CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_

#include <string>
#include <map>
#include <iostream>

#include <torasu/torasu.hpp>

namespace torasu::tools {

class SimpleRenderable : public Renderable {
private:
	bool acceptData, acceptElements;
	std::string typeIdent;

protected:
	explicit SimpleRenderable(std::string typeIdent, bool acceptData = true, bool acceptElements = true);
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) = 0;

public:
	virtual ~SimpleRenderable();

	// Auto-managed
	virtual std::string getType();
	virtual RenderResult* render(RenderInstruction* ri);

	// Overwrite when accepting data
	virtual DataResource* getData();
	virtual void setData(DataResource* data);

	// Implement when accepting elements
	virtual std::map<std::string, Element*> getElements();
	virtual void setElement(std::string key, Element* elem);

	// Auto-managed, overwrite to get more granular control over mass-setting of element linkage and data 
	virtual void setData(DataResource* data,
						 std::map<std::string, Element*> elements);

};

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
