#ifndef STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
#define STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/tools.hpp>
#include <torasu/std/DPNum.hpp>

namespace torasu::tstd {

class RMultiply : public torasu::Renderable {
private:
	std::string ident = std::string("STD::RMULTIPLY");
	const std::string pipeline = std::string("STD::PNUM");

	Renderable* a = NULL;
	Renderable* b = NULL;
	tools::RenderInstructionBuilder rib;
	tools::RenderResultSegmentHandle<DPNum> resHandle;

public:

	explicit RMultiply(Renderable* a, Renderable* b);
	virtual ~RMultiply();

	virtual std::string getType();
	virtual DataResource* getData();
	virtual std::map<std::string, Element*> getElements();

	virtual void setData(DataResource* data,
						 std::map<std::string, Element*> elements);
	virtual void setData(DataResource* data);
	virtual void setElement(std::string key, Element* elem);

	virtual RenderResult* render(RenderInstruction* ri);

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
