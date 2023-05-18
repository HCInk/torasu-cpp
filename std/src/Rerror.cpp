#include "../include/torasu/std/Rerror.hpp"

#include <memory>

#include <torasu/render_tools.hpp>
#include <torasu/std/pipeline_names.hpp>

namespace torasu::tstd {

Rerror::Rerror(torasu::tstd::StringSlot msg)
	: SimpleRenderable(false, true), msgRnd(msg) {}

Rerror::~Rerror() {}

torasu::Identifier Rerror::getType() {
	return "STD::RERROR";
}

torasu::RenderResult* Rerror::render(torasu::RenderInstruction* ri) {
	bool hasMsg = false;
	LogId msgTag;

	tools::RenderHelper rh(ri);
	auto li = rh.li;
	if (li.level <= ERROR) {
		torasu::ResultSettings rs(TORASU_STD_PL_STRING, torasu::tools::NO_FORMAT);
		std::unique_ptr<torasu::RenderResult> rr(rh.runRender(msgRnd.get(), &rs));

		auto msgRes = rh.evalResult<torasu::tstd::Dstring>(rr.get());

		torasu::LogEntry* logEntry;
		if (msgRes) {
			auto* dstr = msgRes.getResult();
			logEntry = new torasu::LogMessage(ERROR, dstr->getString());
		} else {
			logEntry = new torasu::LogMessage(ERROR, "Failed to get error message!");
		}
		msgTag = logEntry->addTag(li.logger);
		li.logger->log(logEntry);
		hasMsg = true;
	}

	return new RenderResult(torasu::RenderResultStatus_INTERNAL_ERROR,
	hasMsg ? new torasu::LogInfoRef(new std::vector<std::vector<LogId>>({{msgTag}})) : nullptr);
}

torasu::ElementMap Rerror::getElements() {
	torasu::ElementMap em;
	em["msg"] = msgRnd;
	return em;
}

const torasu::OptElementSlot Rerror::setElement(std::string key, const ElementSlot* elem) {
	if (key == "msg") return torasu::tools::trySetRenderableSlot(&msgRnd, elem);
	return nullptr;
}

} // namespace torasu::tstd
