#ifndef STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Rstring.hpp>

namespace torasu::tstd {

class Rnet_file : public tools::SimpleRenderable {
private:
	std::string pipeline = std::string(TORASU_STD_PL_FILE);

	tools::ManagedSlot<StringSlot> urlRnd;
	tools::ManagedSlot<StringSlot> headersRnd;

protected:
	ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	explicit Rnet_file(StringSlot url, StringSlot headers = StringSlot());
	~Rnet_file();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
