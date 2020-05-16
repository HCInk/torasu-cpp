#ifndef STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

class Rnet_file : public tools::SimpleRenderable {
private:
	std::string pipeline = std::string(TORASU_STD_PL_FILE);

	std::string url;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	explicit Rnet_file(std::string url);
	virtual ~Rnet_file();

	virtual DataResource* getData();
	virtual void setData(DataResource* data);

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
