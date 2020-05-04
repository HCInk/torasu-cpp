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

	torasu::DataResource* valdr;

public:

	explicit RNum(double val);
	virtual ~RNum();

	virtual std::string getType();
	virtual torasu::DataResource* getData();
	virtual std::map<std::string, Element*> getElements();

	virtual void setData(torasu::DataResource* data,
						 std::map<std::string, Element*> elements);
	virtual void setData(torasu::DataResource* data);
	virtual void setElement(std::string key, Element* elem);

	virtual torasu::RenderResult* render(torasu::RenderInstruction* ri);

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
