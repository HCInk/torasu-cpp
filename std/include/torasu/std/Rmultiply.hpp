#ifndef STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
#define STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class DLL_EXPORT Rmultiply : public tools::SimpleRenderable {
private:
	const std::string numPipeline = std::string(TORASU_STD_PL_NUM);
	const std::string visPipeline = std::string(TORASU_STD_PL_VIS);

	Renderable* a = NULL;
	Renderable* b = NULL;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	Rmultiply(Renderable* a, Renderable* b);
	virtual ~Rmultiply();

	virtual std::map<std::string, Element*> getElements();
	virtual void setElement(std::string key, Element* elem);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
