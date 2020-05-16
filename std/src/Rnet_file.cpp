#include "../include/torasu/std/Rnet_file.hpp"

#include <torasu/std/Dfile.hpp>

#include <curl/curl.h>

namespace torasu::tstd {

Rnet_file::Rnet_file(std::string url) 
	: SimpleRenderable("STD::RNET_FILE", true, false) {
	this->url = url;
}

Rnet_file::~Rnet_file() {}

size_t Rnet_file_WRITE_FUNC(void *ptr, size_t size, size_t nmemb,  std::string *s) {
  size_t new_len = size*nmemb;
  char* data = static_cast<char*>(ptr);
  std::string new_data(data, new_len);
  *s += new_data;
  return size*nmemb;
}


ResultSegment* Rnet_file::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline().compare(pipeline) == 0) {

		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		if(!curl){
			return new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
		}

		std::string dataout;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Rnet_file_WRITE_FUNC);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataout);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		size_t size = dataout.size();
		
		char* data = dataout.data();

		Dfile* file = new Dfile(new std::vector<uint8_t>(data, data+size));
		
		return new ResultSegment(ResultSegmentStatus_OK, file, true);
	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}
}

DataResource* Rnet_file::getData() {
	return NULL; // TODO Rnet_file getData()
}

void Rnet_file::setData(DataResource* data) {
	// TODO Rnet_file setData()
}

} // namespace torasu::tstd