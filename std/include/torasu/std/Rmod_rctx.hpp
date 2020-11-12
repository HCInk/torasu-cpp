#ifndef STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_
#define STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring_pair.hpp>

namespace torasu::tstd {

class Rmod_rctx : public torasu::Renderable,
	public torasu::tools::NamedIdentElement,
	public torasu::tools::SimpleDataElement,
	public torasu::tools::ReadylessElement {
private:
	Dstring_pair data;
	Renderable* mainRnd;
	Renderable* valueRnd;

protected:
	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

public:
	Rmod_rctx(Renderable* main, Renderable* value, std::string rctxKey, std::string valuePipeline);
	~Rmod_rctx();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_
