#ifndef STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_
#define STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_

#include <string>
#include <memory>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Dstring_pair.hpp>

namespace torasu::tstd {

class Rjson_prop : public torasu::tools::SimpleRenderable {
private:
	std::unique_ptr<torasu::tstd::Dstring_pair> config;
	torasu::tools::ManagedRenderableSlot jsonRnd;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rjson_prop(std::string path, torasu::tools::RenderableSlot jsonRnd, bool optional=false);
	~Rjson_prop();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_
