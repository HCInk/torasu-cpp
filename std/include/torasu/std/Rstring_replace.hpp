#ifndef STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rstring_replace : public torasu::tools::SimpleRenderable {
private:
	Renderable* srcRnd;
	Renderable* beforeRnd;
	Renderable* afterRnd;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rstring_replace(Renderable* src, Renderable* before, Renderable* after);
	~Rstring_replace();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_
