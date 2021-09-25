#ifndef STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_
#define STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/Dstring_pair.hpp>

namespace torasu::tstd {

class Rmod_rctx : public torasu::tools::SimpleRenderable {
private:
	Dstring_pair data;
	tools::ManagedRenderableSlot mainRnd;
	tools::ManagedRenderableSlot valueRnd;

public:
	Rmod_rctx(tools::RenderableSlot main, tools::RenderableSlot value, std::string rctxKey, std::string valuePipeline);
	~Rmod_rctx();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_
