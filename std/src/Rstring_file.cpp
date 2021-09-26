#include "../include/torasu/std/Rstring_file.hpp"

#include <memory>

#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Dfile.hpp>

namespace torasu::tstd {

Rstring_file::Rstring_file(torasu::tools::RenderableSlot src)
	: SimpleRenderable(false, true), srcRnd(src) {}

Rstring_file::~Rstring_file() {}

Identifier Rstring_file::getType() {
	return "STD::RSTR_FILE";
}

torasu::RenderResult* Rstring_file::render(torasu::RenderInstruction* ri) {
	auto pipeline = ri->getResultSettings()->getPipeline();
	if (pipeline == TORASU_STD_PL_FILE) {
		tools::RenderHelper rh(ri);

		torasu::ResultSettings strSetting(TORASU_STD_PL_STRING, nullptr);
		std::unique_ptr<RenderResult> rr(rh.runRender(srcRnd, &strSetting));

		auto res = rh.evalResult<tstd::Dstring>(rr.get());

		if (!res) {
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide source for string-file.", res.takeInfoTag());

			return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
		}

		const auto& str = res.getResult()->getString();
		size_t strSize = str.length();
		const char* cstr = str.c_str();

		auto* file = new torasu::tstd::Dfile(strSize);

		uint8_t* data = file->getFileData();
		std::copy(cstr, cstr+strSize, data);

		return rh.buildResult(file);

	} else if (pipeline == TORASU_STD_PL_STRING) {
		tools::RenderHelper rh(ri);

		torasu::ResultSettings fileSetting(TORASU_STD_PL_FILE, nullptr);
		std::unique_ptr<torasu::RenderResult> rr(rh.runRender(srcRnd.get(), &fileSetting));

		auto res = rh.evalResult<tstd::Dfile>(rr.get());

		if (!res) {
			if (rh.mayLog(torasu::WARN))
				rh.lrib.logCause(torasu::WARN, "Failed to provide source for file.", res.takeInfoTag());

			return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
		}

		std::string str(reinterpret_cast<char*>(res.getResult()->getFileData()), res.getResult()->getFileSize());

		return rh.buildResult(new torasu::tstd::Dstring(str));
	} else {
		return new torasu::RenderResult(torasu::RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rstring_file::getElements() {
	torasu::ElementMap elemMap;

	elemMap["src"] = srcRnd.get();

	return elemMap;
}

void Rstring_file::setElement(std::string key, Element* elem) {
	if (torasu::tools::trySetRenderableSlot("src", &srcRnd, false, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd