#include "../include/torasu/std/Rnet_file.hpp"

#if __EMSCRIPTEN__
	#include <emscripten/fetch.h>
#else
	#include <curl/curl.h>
#endif
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
	: SimpleRenderable(false, true),
	  urlRnd(url),  headersRnd(headers) {}

Rnet_file::~Rnet_file() {}

Identifier Rnet_file::getType() {
	return "STD::RNET_FILE";
}

size_t Rnet_file_WRITE_FUNC(void* ptr, size_t size, size_t nmemb,  std::string* s) {
	size_t new_len = size*nmemb;
	char* data = static_cast<char*>(ptr);
	std::string new_data(data, new_len);
	*s += new_data;
	return size*nmemb;
}

RenderResult* Rnet_file::render(RenderInstruction* ri) {

	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_FILE) {

		// Getting url

		tools::RenderHelper rh(ri);
		torasu::ResultSettings resSetting(TORASU_STD_PL_STRING, torasu::tools::NO_FORMAT);

		auto renderId = rh.enqueueRender(urlRnd, &resSetting);
		auto renderIdHeaders = headersRnd.get() != nullptr ? rh.enqueueRender(headersRnd, &resSetting) : 0;

		std::string url;
		{
			std::unique_ptr<torasu::RenderResult> rndRes(rh.fetchRenderResult(renderId));

			auto fetchedRes = rh.evalResult<tstd::Dstring>(rndRes.get());

			if (!fetchedRes) {
				rh.lrib.logCause(LogLevel::WARN, "Error fetching url!", fetchedRes.takeInfoTag());
				return rh.buildResult(torasu::RenderResultStatus_INTERNAL_ERROR);
			}

			url = fetchedRes.getResult()->getString();
		}

		std::string headers = "";
		if (headersRnd.get() != nullptr) {
			std::unique_ptr<torasu::RenderResult> rndRes(rh.fetchRenderResult(renderIdHeaders));

			auto fetchedRes = rh.evalResult<tstd::Dstring>(rndRes.get());

			if (fetchedRes) {
				headers = fetchedRes.getResult()->getString();
			} else {
				if (rh.mayLog(WARN)) {
					rh.lrib.logCause(LogLevel::WARN, "Failed to provide headers, will skip headers.", fetchedRes.takeInfoTag());
				}
			}

		}

		auto headerList = split(headers, "\n");

		// Running Download
		size_t size = 0;
		Dfile* file = nullptr;
#if __EMSCRIPTEN__
		// XXX Untested
		size_t headerCount = headerList.size();
		const char* headerArray[headerCount*2+1];
		std::vector<std::string> headerStringsVec(headerCount*2);
		{
			std::string* headersRaw = headerList.data();
			std::string* headerStrings = headerStringsVec.data();
			size_t headerArrIndex = 0;
			for (size_t i = 0; i < headerCount; i++) {
				std::string rawHeader = headersRaw[i];
				size_t found = rawHeader.find(": ");
				if (found >= rawHeader.size()) continue;
				auto& key = headerStrings[headerArrIndex] = rawHeader.substr(0, found);
				headerArray[headerArrIndex] = key.c_str();
				headerArrIndex++;
				auto& value = headerStrings[headerArrIndex] = rawHeader.substr(found+2);
				headerArray[headerArrIndex] = value.c_str();
				headerArrIndex++;
			}
			headerArray[headerArrIndex] = 0x00;
		}

		emscripten_fetch_attr_t attr;
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "GET");
		attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
		attr.requestHeaders = headerArray;
		emscripten_fetch_t* fetch = emscripten_fetch(&attr, url.c_str()); // Blocks here until the operation is complete.
		if (fetch->status == 200) {
			// printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
			// The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];

			size = fetch->numBytes;
			file = new Dfile(size);
			copy(fetch->data, fetch->data+size, file->getFileData());
		} else {
			printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
			throw std::runtime_error("Downloading " + std::string(fetch->url) + " failed, HTTP failure status code: " + std::to_string(fetch->status));
		}
		emscripten_fetch_close(fetch);
#else
		CURL* curl;
		CURLcode res;
		curl = curl_easy_init();
		if (!curl)
			throw std::runtime_error("Failed to init curl!");

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

		size = dataout.size();
		const char* data = dataout.data();
		file = new Dfile(size);
		copy(data, data+size, file->getFileData());
#endif


		torasu::tools::log_checked(rh.li, LogLevel::DEBUG,
								   "Loaded net-file \"" + url + "\" (" + std::to_string(size) + "byte)");

		return rh.buildResult(file);
	} else {
		return new RenderResult(RenderResultStatus_INVALID_SEGMENT);
	}
}

torasu::ElementMap Rnet_file::getElements() {
	torasu::ElementMap elems;

	elems["url"] = urlRnd;
	elems["headers"] = headersRnd;

	return elems;
}

const torasu::OptElementSlot Rnet_file::setElement(std::string key, const torasu::ElementSlot* elem) {
	if (key == "url") return torasu::tools::trySetRenderableSlot(&urlRnd, elem);
	if (key == "headers") return torasu::tools::trySetRenderableSlot(&headersRnd, elem);
	return nullptr;
}

} // namespace torasu::tstd