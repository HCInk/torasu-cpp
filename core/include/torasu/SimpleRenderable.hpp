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

	virtual std::string getType();

	virtual DataResource* getData();

	virtual std::map<std::string, Element*> getElements();

	virtual void resetElements();

	virtual void setData(DataResource* data);

	virtual void setElement(std::string key, Element* elem);

	virtual void setData(DataResource* data,
						 std::map<std::string, Element*> elements);

	virtual RenderResult* render(RenderInstruction* ri);

};

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_SIMPLERENDERABLE_HPP_
