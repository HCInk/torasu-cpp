#include "../include/torasu/std/DRBImg.hpp"

namespace torasu::tstd {

DRBImg::DRBImg(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	this->data = new uint8_t[width*height*3];
}

DRBImg::~DRBImg() {
	delete[] data;
}

std::string DRBImg::getIdent() {
	return ident;
}

DataDump* DRBImg::getData() {
	return NULL; // TODO DataDump of DRImg
}

uint32_t DRBImg::getWidth() {
	return width;
}

uint32_t DRBImg::getHeight() {
	return height;
}

uint8_t* DRBImg::getImageData() {
	return data;
}

} // namespace torasu::tstd