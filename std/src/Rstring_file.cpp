#include "../include/torasu/std/Rstring_file.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Dfile.hpp>

namespace torasu::tstd {

Rstring_file::Rstring_file(torasu::tools::RenderableSlot src)
	: SimpleRenderable("STD::RSTR_FILE", false, true), srcRnd(src) {
}

Rstring_file::~Rstring_file() {}

torasu::ResultSegment* Rstring_file::renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) {
	std::string pipeline = resSettings->getPipeline();
	if (pipeline == TORASU_STD_PL_FILE) {
		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();
		auto li = ri->getLogInstruction();

		torasu::tools::RenderInstructionBuilder strRib;
		auto strHandle = strRib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		std::unique_ptr<RenderResult> rr(strRib.runRender(srcRnd.get(), rctx, ei, li));

		auto* res = strHandle.getFrom(rr.get()).getResult();

		if (res == nullptr) {
			if (li.level <= torasu::LogLevel::ERROR)
				li.logger->log(torasu::LogLevel::ERROR, "Failed to provide source for string-file.");

			return new torasu::ResultSegment(torasu::ResultSegmentStatus_INTERNAL_ERROR);
		}


		size_t strSize = res->getString().length();
		const char* cstr = res->getString().c_str();

		auto* file = new torasu::tstd::Dfile(strSize);

		uint8_t* data = file->getFileData();
		std::copy(cstr, cstr+strSize, data);

		return new torasu::ResultSegment(torasu::ResultSegmentStatus_OK, file, true);
	} else {
		return new torasu::ResultSegment(torasu::ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_file::getElements() {
	torasu::ElementMap elemMap;

	elemMap["list"] = srcRnd.get();

	return elemMap;
}

void Rstring_file::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("src", &srcRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd