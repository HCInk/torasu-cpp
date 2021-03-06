#ifndef STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rstring.hpp>

namespace torasu::tstd {

class Rstring_replace : public torasu::tools::SimpleRenderable {
private:
	tools::ManagedSlot<StringSlot> srcRnd;
	tools::ManagedSlot<StringSlot> beforeRnd;
	tools::ManagedSlot<StringSlot> afterRnd;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rstring_replace(StringSlot src, StringSlot before, StringSlot after);
	~Rstring_replace();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RSTRING_REPLACE_HPP_
