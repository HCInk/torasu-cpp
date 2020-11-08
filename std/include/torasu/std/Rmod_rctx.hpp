#ifndef STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_
#define STD_INCLUDE_TORASU_STD_RMOD_RCTX_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring.hpp>

namespace torasu::tstd {

class Dmod_rctx_data : public torasu::DataPackable {
private:
	std::string rctxKey;
	std::string valuePipeline;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	Dmod_rctx_data(std::string jsonStripped);
	Dmod_rctx_data(torasu::json jsonParsed);

	Dmod_rctx_data(std::string rctxKey, std::string valuePipeline);
	~Dmod_rctx_data();

	std::string getIdent() override;
	
	std::string getRctxKey();
	std::string getValuePipeline();
};

class Rmod_rctx : public torasu::Renderable,
	public torasu::tools::NamedIdentElement,
	public torasu::tools::SimpleDataElement,
	public torasu::tools::ReadylessElement {
private:
	Renderable* mainRnd;
	Renderable* valueRnd;
	Dmod_rctx_data data;

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
