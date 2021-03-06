#ifndef STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_
#define STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

namespace torasu::tstd {

class Rproperty : public torasu::tools::SimpleRenderable {
private:
	tools::ManagedRenderableSlot propertySrc;
	std::string fromProperty;
	std::string servedPipeline;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rproperty(tools::RenderableSlot propertySrc, std::string fromProperty, std::string servedPipeline);
	~Rproperty();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RPROPERTY_HPP_
