#include "../include/torasu/std/Dmatrix.hpp"

#include <string>

#include <torasu/json.hpp>

namespace torasu::tstd {

void Dmatrix::initBuffer(size_t size, size_t height, std::initializer_list<torasu::tstd::Dnum>* init) {
	this->height = height == 0 ? size : height;
	this->width = size/height;
	if (init) this->nums = new std::vector<torasu::tstd::Dnum>(*init);
	else this->nums = new std::vector<torasu::tstd::Dnum>(width*height);
}

Dmatrix::Dmatrix(std::string jsonStripped) : DataPackable(jsonStripped) {
	ensureLoaded();
}
Dmatrix::Dmatrix(torasu::json jsonParsed) : DataPackable(jsonParsed) {
	ensureLoaded();
}

Dmatrix::Dmatrix(std::initializer_list<torasu::tstd::Dnum> numbers, size_t height) {
	initBuffer(numbers.size(), height, &numbers);
}

Dmatrix::Dmatrix(const Dmatrix& original)
	: height(original.getHeight()),
	  width(original.getWidth()),
	  nums(new std::vector<torasu::tstd::Dnum>(original.getNums(), original.getNums()+(width*height))) {}

Dmatrix::~Dmatrix() {
	if (nums) delete nums;
}

torasu::tstd::Dnum* Dmatrix::getNums() const {
	return nums->data();
}

size_t Dmatrix::getWidth() const {
	return width;
}

size_t Dmatrix::getHeight() const {
	return height;
}

std::string Dmatrix::getIdent() const {
	return "STD::DMATRIX";
}

void Dmatrix::load() {
	torasu::json json = getJson();
	torasu::json foundData = json["data"];
	torasu::json foundHeight = json["height"];
	initBuffer(foundData.size(), foundHeight);
	for (size_t i = 0; i < height*width; i++) {
		(*nums)[i] = foundData[i];
	}
}

torasu::json Dmatrix::makeJson() {
	std::vector<torasu::json> dataArr(height*width);
	for (size_t i = 0; i < dataArr.size(); i++) {
		dataArr[i] = (*nums)[i].getNum();
	}
	torasu::json json;
	json["d"] = dataArr;
	json["h"] = height;
	return json;
}

Dmatrix* Dmatrix::clone() const {
	return new Dmatrix(*this);
}

} // namespace torasu::tstd
