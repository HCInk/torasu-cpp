#ifndef STD_INCLUDE_TORASU_STD_RDIVIDE_HPP_
#define STD_INCLUDE_TORASU_STD_RDIVIDE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/spoilsD.hpp>
#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rdivide : public tools::SimpleRenderable {
private:
	const std::string numPipeline = std::string(TORASU_STD_PL_NUM);
	const std::string visPipeline = std::string(TORASU_STD_PL_VIS);

	tools::ManagedSlot<NumSlot> a;
	tools::ManagedSlot<NumSlot> b;

protected:
	ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	Rdivide(NumSlot a, NumSlot b);
	~Rdivide();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RDIVIDE_HPP_
