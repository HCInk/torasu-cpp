#include "../include/torasu/std/Dbimg.hpp"

namespace torasu::tstd {

Dbimg::Dbimg(DRBImg_FORMAT format) : Dbimg(format.getWidth(), format.getHeight()) {}

Dbimg::Dbimg(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	this->bufferSize = width*height*4;
	this->data = new uint8_t[bufferSize];
}

Dbimg::~Dbimg() {
	delete[] data;
}

uint64_t Dbimg::getBufferSize() {
	return bufferSize;
}

std::string Dbimg::getIdent() {
	return ident;
}

DataDump* Dbimg::getData() {
	return NULL; // TODO DataDump of DRImg
}

uint32_t Dbimg::getWidth() {
	return width;
}

uint32_t Dbimg::getHeight() {
	return height;
}

uint8_t* Dbimg::getImageData() {
	return data;
}

DRBImg_FORMAT::DRBImg_FORMAT(std::string jsonStripped) : DataPackable(jsonStripped) {}
DRBImg_FORMAT::DRBImg_FORMAT(nlohmann::json jsonParsed) : DataPackable(jsonParsed) {}

DRBImg_FORMAT::DRBImg_FORMAT(u_int32_t width, u_int32_t height) {
	this->width = width;
	this->height = height;
}

std::string DRBImg_FORMAT::getIdent() {
	return ident;
}

void DRBImg_FORMAT::load() {
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

nlohmann::json DRBImg_FORMAT::makeJson() {
	return {
		{"w", width},
		{"h", height}
	};
}

u_int32_t DRBImg_FORMAT::getWidth() {
	return width;
}

u_int32_t DRBImg_FORMAT::getHeight() {
	return height;
}

} // namespace torasu::tstd