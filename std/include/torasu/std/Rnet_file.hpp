#ifndef STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

class Rnet_file : public tools::SimpleRenderable {
private:
	std::string pipeline = std::string(TORASU_STD_PL_FILE);

	Renderable* urlRnd;
	bool ownsUrl;

protected:
	virtual ResultSegment* renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) override;

public:
	explicit Rnet_file(std::string url);
	explicit Rnet_file(Renderable* url);
	virtual ~Rnet_file();

	std::map<std::string, Element*> getElements() override;
	void setElement(std::string key, Element* elem) override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
