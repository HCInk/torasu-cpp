#ifndef STD_INCLUDE_TORASU_STD_RNUM_HPP_
#define STD_INCLUDE_TORASU_STD_RNUM_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>

namespace torasu::tstd {

class RNum : public torasu::Renderable {
private:
	std::string ident = std::string("STD::RNUM");
	std::string pipeline = std::string("STD::PNUM");

	DataResource* valdr;

public:

	explicit RNum(double val);
	virtual ~RNum();

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

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
