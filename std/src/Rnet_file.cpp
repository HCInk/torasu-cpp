#include "../include/torasu/std/Rnet_file.hpp"

#include <curl/curl.h>

#include <torasu/render_tools.hpp>
#include <torasu/log_tools.hpp>

#include <torasu/std/Dfile.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/Rstring.hpp>

using namespace std;

namespace {

inline std::vector<std::string> split(std::string base, std::string delimiter) {
	std::vector<std::string> final;
	size_t pos = 0;
	std::string token;
	while ((pos = base.find(delimiter)) != std::string::npos) {
		token = base.substr(0, pos);
		final.push_back(token);
		base.erase(0, pos + delimiter.length());
	}
	final.push_back(base);
	return final;
}

} // namespace


namespace torasu::tstd {

Rnet_file::Rnet_file(StringSlot url, StringSlot headers)
	: SimpleRenderable("STD::RNET_FILE", false, true),
	  urlRnd(url),  headersRnd(headers) {}

Rnet_file::~Rnet_file() {}

size_t Rnet_file_WRITE_FUNC(void* ptr, size_t size, size_t nmemb,  std::string* s) {
	size_t new_len = size*nmemb;
	char* data = static_cast<char*>(ptr);
	std::string new_data(data, new_len);
	*s += new_data;
	return size*nmemb;
}

ResultSegment* Rnet_file::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline().compare(pipeline) == 0) {

		// Getting url

		tools::RenderHelper rh(ri);

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		auto renderId = rib.enqueueRender(urlRnd, &rh);
		auto renderIdHeaders = headersRnd.get() != nullptr ? rib.enqueueRender(headersRnd, &rh) : 0;

		std::string url;
		{
			std::unique_ptr<torasu::RenderResult> rndRes(rh.fetchRenderResult(renderId));

			auto fetchedRes = segHandle.getFrom(rndRes.get(), &rh);

			if (fetchedRes.getResult() == nullptr) {
				rh.lrib.logCause(LogLevel::WARN, "Error fetching url!", fetchedRes.takeInfoTag());
				return rh.buildResult(torasu::ResultSegmentStatus_INTERNAL_ERROR);
			}

			url = fetchedRes.getResult()->getString();
		}

		std::string headers = "";
		if (headersRnd.get() != nullptr) {
			std::unique_ptr<torasu::RenderResult> rndRes(rh.fetchRenderResult(renderIdHeaders));

			auto fetchedRes = segHandle.getFrom(rndRes.get(), &rh);

			if (fetchedRes.getResult() != nullptr) {
				headers = fetchedRes.getResult()->getString();
			} else {
				if (rh.mayLog(WARN)) {
					rh.lrib.logCause(LogLevel::WARN, "Failed to provide headers, will skip headers.", fetchedRes.takeInfoTag());
				}
			}

		}

		// Running Download

		CURL* curl;
		CURLcode res;
		curl = curl_easy_init();
		if (!curl)
			throw std::runtime_error("Failed to init curl!");

		auto headerList = split(headers, "\n");

		struct curl_slist* curlHeaders = nullptr;

		for (auto header : headerList) {
			if (header.length() > 0) {
				curlHeaders = curl_slist_append(curlHeaders, header.c_str());
			}
		}

		std::string dataout;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Rnet_file_WRITE_FUNC);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataout);

		res = curl_easy_perform(curl);

		curl_slist_free_all(curlHeaders);
		curl_easy_cleanup(curl);

		if (res != CURLcode::CURLE_OK)
			throw std::runtime_error("Aborted due to CURL error - code: " + std::to_string(res));

		size_t size = dataout.size();
		const char* data = dataout.data();

		Dfile* file = new Dfile(size);
		copy(data, data+size, file->getFileData());

		torasu::tools::log_checked(rh.li, LogLevel::DEBUG,
								   "Loaded net-file \"" + url + "\" (" + std::to_string(size) + "byte)");

		return rh.buildResult(file);
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rnet_file::getElements() {
	torasu::ElementMap elems;

	elems["url"] = urlRnd.get();
	if (urlRnd.get() != nullptr) elems["headers"] = headersRnd.get();

	return elems;
}

void Rnet_file::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("url", &urlRnd, false, key, elem)) return;
	if (torasu::tools::trySetRenderableSlot("headers", &headersRnd, true, key, elem)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd