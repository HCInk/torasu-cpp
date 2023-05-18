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

public:
	explicit Rfallback(std::vector<torasu::RenderableSlot> slots);
	~Rfallback();
	Identifier getType() override;

	RenderResult* render(RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;

};


} // namespace torasu::tstd



#endif // STD_INCLUDE_TORASU_STD_RFALLBACK_HPP_
