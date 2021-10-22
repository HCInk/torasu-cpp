#include "../include/torasu/std/Dbimg.hpp"

#include <algorithm>

using namespace std;

#define IDENT "STD::DBIMG"

namespace torasu::tstd {

//
//	Dbimg
//

Dbimg::Dbimg(Dbimg_FORMAT format) : Dbimg(format.getWidth(), format.getHeight()) {}

Dbimg::Dbimg(uint32_t width, uint32_t height, Dbimg::CropInfo* cropInfo) {
	this->width = width;
	this->height = height;
	this->cropInfo = cropInfo;
	this->data = new uint8_t[width*height*4];
}

Dbimg::Dbimg(const Dbimg& copy)
	: Dbimg(copy.width, copy.height, new Dbimg::CropInfo(*copy.cropInfo)) {
	std::copy(copy.data, copy.data+(width*height*4), this->data);
}

Dbimg::~Dbimg() {
	delete[] data;
	if (cropInfo != nullptr) delete cropInfo;
}

void Dbimg::clear() {
	uint32_t* data32 = reinterpret_cast<uint32_t*>(data);
	std::fill(data32, data32+(width*height), 0x00);
}

torasu::Identifier Dbimg::getType() const {
	return IDENT;
}

DataDump* Dbimg::dumpResource() {
	return nullptr; // TODO DataDump of Rbimg
}

Dbimg* Dbimg::clone() const {
	return new Dbimg(*this);
}

//
//	Dbimg_FORMAT
//

Dbimg_FORMAT::Dbimg_FORMAT(uint32_t width, uint32_t height, Dbimg::CropInfo* cropInfo)  : ResultFormatSettings(IDENT), width(width), height(height), cropInfo(cropInfo) {}

Dbimg_FORMAT::Dbimg_FORMAT(const torasu::json& jsonParsed) : ResultFormatSettings(IDENT), DataPackable(jsonParsed) {}
Dbimg_FORMAT::Dbimg_FORMAT(const std::string& jsonStripped) : ResultFormatSettings(IDENT), DataPackable(jsonStripped) {}
Dbimg_FORMAT::~Dbimg_FORMAT() {
	if (cropInfo != nullptr) delete cropInfo;
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

torasu::json Dbimg_FORMAT::makeJson() {
	return {
		{"w", width},
		{"h", height}
	};
}

Dbimg_FORMAT* Dbimg_FORMAT::clone() const {
	return new Dbimg_FORMAT(*this);
}

} // namespace torasu::tstd