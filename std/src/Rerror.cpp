#include "../include/torasu/std/Rerror.hpp"

#include <memory>

#include <torasu/render_tools.hpp>
#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

Rerror::Rerror(torasu::tstd::StringSlot msg)
	: NamedIdentElement("STD::RERROR"), SimpleDataElement(false, true), msgRnd(msg) {}

Rerror::~Rerror() {}


torasu::RenderResult* Rerror::render(torasu::RenderInstruction* ri) {
	auto li = ri->getLogInstruction();
	bool hasMsg = false;
	LogId msgTag;

	if (li.level <= ERROR) {
		auto* ei = ri->getExecutionInterface();
		torasu::tools::RenderInstructionBuilder rib;
		auto strHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		std::unique_ptr<torasu::RenderResult> rr(rib.runRender(msgRnd.get(), ri->getRenderContext(), ei, li));

		auto msgRes = strHandle.getFrom(rr.get());

		torasu::LogEntry* logEntry;
		if (msgRes.getResult() != nullptr) {
			auto* dstr = msgRes.getResult();
			logEntry = new torasu::LogMessage(ERROR, dstr->getString());
		} else {
			logEntry = new torasu::LogMessage(ERROR, "Failed to get error message!");
		}
		msgTag = logEntry->addTag(li.logger);
		li.logger->log(logEntry);
		hasMsg = true;
	}

	auto* results = new std::map<std::string, ResultSegment*>();
	for (const auto& setting : *ri->getResultSettings()) {
		(*results)[setting->getKey()] =
			new ResultSegment(torasu::ResultSegmentStatus_INTERNAL_ERROR,
		hasMsg ? new torasu::LogInfoRef(new std::vector<std::vector<LogId>>({{msgTag}})) : nullptr);
	}

	return new torasu::RenderResult(torasu::ResultStatus_PARTIAL_ERROR, results);


}

torasu::ElementMap Rerror::getElements() {
	torasu::ElementMap em;
	em["msg"] = msgRnd.get();
	return em;
}

void Rerror::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("msg", &msgRnd, true, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd
