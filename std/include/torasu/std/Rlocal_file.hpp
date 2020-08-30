#ifndef STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_

#include <string>
#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class DLL_EXPORT Rlocal_file : public tools::SimpleRenderable {
private:
	std::string pipeline = std::string(TORASU_STD_PL_FILE);

	std::string path;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	explicit Rlocal_file(std::string path);
	virtual ~Rlocal_file();

	virtual DataResource* getData();
	virtual void setData(DataResource* data);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_
