#ifndef STD_INCLUDE_TORASU_STD_RSIN_HPP_
#define STD_INCLUDE_TORASU_STD_RSIN_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rsin : public tools::SimpleRenderable {
private:
	Renderable* valRnd;

protected:
	ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	Rsin(Renderable* val);
	~Rsin();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSIN_HPP_
