#ifndef STD_INCLUDE_TORASU_STD_RFALLBACK_HPP_
#define STD_INCLUDE_TORASU_STD_RFALLBACK_HPP_

#include <string>
#include <vector>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rfallback : public torasu::tools::SimpleRenderable {
private:
	std::map<size_t, torasu::tools::ManagedRenderableSlot> slots;

protected:
	ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	explicit Rfallback(std::vector<torasu::tools::RenderableSlot> slots);
	~Rfallback();

	torasu::ElementMap getElements();
	void setElement(std::string key, torasu::Element* elem);

};


} // namespace torasu::tstd



#endif // STD_INCLUDE_TORASU_STD_RFALLBACK_HPP_
