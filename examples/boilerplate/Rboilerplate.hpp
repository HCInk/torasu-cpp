#ifndef EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_
#define EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include "Dboilerplate.hpp"

namespace torasu::examples {

class Rboilerplate : public torasu::tools::SimpleRenderable {
private:
    Dboilerplate* data;
	Renderable* exampleRnd;

protected:
    virtual torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri);

public:
	Rboilerplate(Dboilerplate* data, Renderable* exampleRnd);
    virtual ~Rboilerplate();

	std::map<std::string, Element*> getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::examples

#endif // EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_