#ifndef EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_
#define EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include "Dboilerplate.hpp"

namespace torasu::texample {

class Rboilerplate : public torasu::tools::SimpleRenderable {
private:
	Dboilerplate* data;
	Renderable* exampleRnd;

protected:
	torasu::ResultSegment* render(torasu::RenderInstruction* ri) override;

public:
	Rboilerplate(Dboilerplate* data, Renderable* exampleRnd);
	~Rboilerplate();
	Identifier getType() override;

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::texample

#endif // EXAMPLES_BOILERPLATE_RBOILERPLATE_HPP_
