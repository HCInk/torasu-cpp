#ifndef STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_
#define STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_

#include <map>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipelines.hpp>
#include <torasu/std/spoilsDR.hpp>

namespace torasu::tstd {

class RMultiply : public tools::SimpleRenderable {
private:
	const std::string pipeline = std::string(TORASU_STD_PL_NUM);

	Renderable* a = NULL;
	Renderable* b = NULL;
	tools::RenderInstructionBuilder rib;
	tools::RenderResultSegmentHandle<DPNum> resHandle;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri);

public:
	RMultiply(Renderable* a, Renderable* b);
	virtual ~RMultiply();

	virtual std::map<std::string, Element*> getElements();
	virtual void setElement(std::string key, Element* elem);
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMULTIPLY_HPP_