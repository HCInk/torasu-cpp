#include "../include/torasu/std/Rnet_file.hpp"

#include <curl/curl.h>

#include <iostream>

#include <torasu/render_tools.hpp>

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

Rnet_file::Rnet_file(std::string url)
	: SimpleRenderable("STD::RNET_FILE", false, true),
	  urlRnd(new torasu::tstd::Rstring(url)), headersRnd(nullptr), ownsUrl(true) {}

Rnet_file::Rnet_file(Renderable* url, Renderable* headers)
	: SimpleRenderable("STD::RNET_FILE", false, true),
	  urlRnd(url),  headersRnd(headers), ownsUrl(false) {}

Rnet_file::~Rnet_file() {
	if (ownsUrl) delete urlRnd;
}

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

		auto* ei = ri->getExecutionInterface();
		auto* rctx = ri->getRenderContext();

		torasu::tools::RenderInstructionBuilder rib;
		auto segHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

		auto renderId = rib.enqueueRender(urlRnd, rctx, ei);
		auto renderIdHeaders = headersRnd != nullptr ? rib.enqueueRender(headersRnd, rctx, ei) : 0;

		std::string url;
		{
			std::unique_ptr<torasu::RenderResult> rndRes(ei->fetchRenderResult(renderId));

			auto fetchedRes = segHandle.getFrom(rndRes.get());

			if (fetchedRes.getResult() == nullptr) {
				return new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
			}

			url = fetchedRes.getResult()->getString();
		}

		std::string headers = "";
		if (headersRnd != nullptr) {
			std::unique_ptr<torasu::RenderResult> rndRes(ei->fetchRenderResult(renderIdHeaders));

			auto fetchedRes = segHandle.getFrom(rndRes.get());

			if (fetchedRes.getResult() != nullptr) {
				headers = fetchedRes.getResult()->getString();
			} else {
				// TODO Note that the renderable failed to provide the headers
			}

		}

		// Running Download

		CURL* curl;
		CURLcode res;
		curl = curl_easy_init();
		if (!curl) {
			return new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
		}

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

		if (res != CURLcode::CURLE_OK) {
			cerr << "Aborted due to CURL error - code: " << res << endl;
			return new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
		}

		size_t size = dataout.size();
		const char* data = dataout.data();

		Dfile* file = new Dfile(size);
		copy(data, data+size, file->getFileData());

		return new ResultSegment(ResultSegmentStatus_OK, file, true);
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rnet_file::getElements() {
	torasu::ElementMap elems;

	elems["url"] = urlRnd;

	return elems;
}

void Rnet_file::setElement(std::string key, torasu::Element* elem) {
	if (torasu::tools::trySetRenderableSlot("ex", &urlRnd, false, key, elem, &ownsUrl)) return;
	throw torasu::tools::makeExceptSlotDoesntExist(key);
}

} // namespace torasu::tstd