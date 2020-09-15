#ifndef STD_INCLUDE_TORASU_STD_RNUM_HPP_
#define STD_INCLUDE_TORASU_STD_RNUM_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

class Rnum : public tools::SimpleRenderable {
private:
	std::string pipeline = std::string(TORASU_STD_PL_NUM);

	DataResource* valdr;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	explicit Rnum(double val);
	virtual ~Rnum();

	virtual DataResource* getData();
	virtual void setData(DataResource* data);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUM_HPP_
