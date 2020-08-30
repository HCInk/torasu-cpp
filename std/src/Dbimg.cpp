#include "../include/torasu/std/Dbimg.hpp"

using namespace std;

namespace torasu::tstd {

Dbimg::Dbimg(Dbimg_FORMAT format) : Dbimg(format.getWidth(), format.getHeight()) {}

Dbimg::Dbimg(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	this->data = new uint8_t[width*height*4];
}

Dbimg::Dbimg(const Dbimg& copy) {
	this->width = copy.width;
	this->height = copy.height;
	this->data = new uint8_t[width*height*4];
	std::copy(copy.data, copy.data+(width*height*4), this->data);
}

Dbimg::~Dbimg() {
	delete[] data;
}

std::string Dbimg::getIdent() {
	return ident;
}

DataDump* Dbimg::getData() {
	return NULL; // TODO DataDump of DRImg
}

Dbimg_FORMAT::Dbimg_FORMAT(std::string jsonStripped) : DataPackable(jsonStripped) {}
Dbimg_FORMAT::Dbimg_FORMAT(nlohmann::json jsonParsed) : DataPackable(jsonParsed) {}

Dbimg_FORMAT::Dbimg_FORMAT(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
}

std::string Dbimg_FORMAT::getIdent() {
	return ident;
}

void Dbimg_FORMAT::load() {
	auto json = getJson();
	if (json["w"].is_number()) {
		width = json["w"];
	} else {
		width = 0;
	}

	if (json["h"].is_number()) {
		height = json["h"];
	} else {
		height = 0;
	}
}

nlohmann::json Dbimg_FORMAT::makeJson() {
	return {
		{"w", width},
		{"h", height}
	};
}

} // namespace torasu::tstd