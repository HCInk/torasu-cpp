#ifndef STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
#define STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class Rmultiply : public tools::SimpleRenderable {
private:
	const std::string pipeline = std::string(TORASU_STD_PL_NUM);

	Renderable* a = NULL;
	Renderable* b = NULL;
	tools::RenderInstructionBuilder rib;
	tools::RenderResultSegmentHandle<Dnum> resHandle;

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
