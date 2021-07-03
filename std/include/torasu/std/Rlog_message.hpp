#ifndef STD_INCLUDE_TORASU_STD_RLOG_MESSAGE_HPP_
#define STD_INCLUDE_TORASU_STD_RLOG_MESSAGE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rlog_message : public torasu::tools::NamedIdentElement,
	public torasu::tools::SimpleDataElement,
	public torasu::tools::ReadylessElement,
	public torasu::Renderable {
private:
	torasu::LogMessage message;
	torasu::tools::ManagedRenderableSlot srcRnd;

public:
	Rlog_message(torasu::LogMessage message, torasu::tools::RenderableSlot src);
	Rlog_message(torasu::LogLevel level, std::string message, torasu::tools::RenderableSlot src);
	~Rlog_message();

	torasu::ResultSegment* render(RenderInstruction* ri) override;

	DataResource* getData() override;
	void setData(DataResource* data) override;

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RLOG_MESSAGE_HPP_
