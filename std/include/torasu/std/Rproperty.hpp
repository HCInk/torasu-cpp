#ifndef STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_
#define STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rproperty : public torasu::tools::SimpleRenderable {
private:
	Renderable* propertySrc;
	std::string fromProperty;
	std::string servedPipeline;

protected:
    virtual torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri);

public:
	Rproperty(Renderable* propertySrc, std::string fromProperty, std::string servedPipeline);
	virtual ~Rproperty();
    
	std::map<std::string, Element*> getElements() override;
    void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_
