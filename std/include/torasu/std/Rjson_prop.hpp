#ifndef STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_
#define STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class Rjson_prop : public torasu::tools::SimpleRenderable {
private:
	torasu::tstd::Dstring* path;
	Renderable* jsonRnd;

protected:
	virtual torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri);

public:
	Rjson_prop(std::string path, Renderable* jsonRnd);
	virtual ~Rjson_prop();

	std::map<std::string, Element*> getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RJSON_PROP_HPP_
