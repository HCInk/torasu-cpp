#ifndef STD_INCLUDE_TORASU_STD_RNUM_HPP_
#define STD_INCLUDE_TORASU_STD_RNUM_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class RNum : public tools::SimpleRenderable {
private:
	std::string ident = std::string("STD::RNUM");
	std::string pipeline = std::string("STD::PNUM");

	DataResource* valdr;

protected:

	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:

	explicit RNum(double val);
	virtual ~RNum();

	virtual void setData(DataResource* data);



};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
