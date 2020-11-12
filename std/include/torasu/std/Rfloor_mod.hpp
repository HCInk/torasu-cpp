#ifndef STD_INCLUDE_TORASU_STD_RMOD_HPP_
#define STD_INCLUDE_TORASU_STD_RMOD_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rfloor_mod : public torasu::tools::SimpleRenderable {
private:
	Renderable* valRnd;
	Renderable* facRnd;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rfloor_mod(Renderable* val, Renderable* fac);
	~Rfloor_mod();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;


};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RMOD_HPP_
