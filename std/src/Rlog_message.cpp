#include "../include/torasu/std/Rlog_message.hpp"

namespace torasu::tstd {

Rlog_message::Rlog_message(torasu::LogMessage message, torasu::RenderableSlot src)
	: SimpleRenderable(true, true),
	  message(message),
	  srcRnd(src) {}

Rlog_message::Rlog_message(torasu::LogLevel level, std::string message, torasu::RenderableSlot src)
	: SimpleRenderable(true, true),
	  message(level, message),
	  srcRnd(src) {}

Rlog_message::~Rlog_message() {}

Identifier Rlog_message::getType() {
	return "STD::RLOG_MESSAGE";
}

RenderResult* Rlog_message::render(RenderInstruction* ri) {

	auto li = ri->getLogInstruction();

	if (li.level <= message.level) {
		li.logger->log(new LogMessage(message));
	}

	auto ei = ri->getExecutionInterface();

	auto rid = ei->enqueueRender(srcRnd.get(), ri->getRenderContext(), ri->getResultSettings(), li, 0);

	torasu::RenderResult* rr = ei->fetchRenderResult(rid);

	return rr;

}

DataResource* Rlog_message::getData() {
	return nullptr; // TODO conversion from LogEntry to DR
}

void Rlog_message::setData(DataResource* data) {
	// TODO conversion from DR to LogEntry
}

torasu::ElementMap Rlog_message::getElements() {
	torasu::ElementMap elmap;
	elmap["src"] = srcRnd;
	return elmap;
}

const torasu::OptElementSlot Rlog_message::setElement(std::string key, const ElementSlot* elem) {
	if (key == "src") return torasu::tools::trySetRenderableSlot(&srcRnd, elem);
	return nullptr;
}

} // namespace torasu::tstd
