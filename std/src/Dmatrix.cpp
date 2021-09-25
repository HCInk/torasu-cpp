#include "../include/torasu/std/Dmatrix.hpp"

#include <string>
#include <cmath>

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

Dmatrix::Dmatrix(size_t size) {
	initBuffer(size*size, size);
	for (size_t i = 0; i < size; i++) {
		(*nums)[i*(size+1)] = 1;
	}
}

Dmatrix::Dmatrix(size_t width, size_t height) {
	initBuffer(width*height, height);
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

Dmatrix& Dmatrix::operator=(const Dmatrix& other) {
	size_t srcWidth = other.getWidth();

	size_t copyWidth = std::min(srcWidth, width);
	size_t copyHeight = std::min(other.getHeight(), height);
	torasu::tstd::Dnum* destPtr = getNums();
	torasu::tstd::Dnum* srcPtr = other.getNums();

	for (size_t y = 0; y < copyHeight; y++) {
		for (size_t x = 0; x < copyWidth; x++) {
			destPtr[x] = srcPtr[x];
		}
		destPtr += width;
		srcPtr += srcWidth;
	}

	return *this;
}

torasu::Identifier Dmatrix::getType() const {
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

Dmatrix Dmatrix::multiplyByFactor(Dnum num) const {
	Dmatrix mulMatrix(width, height);
	Dnum* destNums = mulMatrix.getNums();
	for (size_t i = 0; i < nums->size(); i++) {
		destNums[i] = (*nums)[i].getNum() * num.getNum();
	}
	return mulMatrix;
}

Dmatrix Dmatrix::multiplyByMatrix(Dmatrix other) const {
	if (width != other.getHeight())
		throw std::invalid_argument("Can't multiply matrices, where the width of A is different to the height of B!");
	Dmatrix mulMatrix(other.getWidth(), height);
	Dnum* aNums = this->getNums();
	Dnum* bNums = other.getNums();
	Dnum* destNums = mulMatrix.getNums();
	for (size_t y = 0; y < mulMatrix.getHeight(); y++) {
		for (size_t x = 0; x < mulMatrix.getWidth(); x++) {
			double total = 0;
			for (size_t i = 0; i < width; i++) {
				total += aNums[i + y*width].getNum() * bNums[x + i*other.getWidth()].getNum();
			}
			(*destNums) = total;
			destNums++;
		}
	}
	return mulMatrix;
}


Dmatrix Dmatrix::transpose() const {
	Dmatrix transposedMatrix(height, width);
	Dnum* destNums = transposedMatrix.getNums();
	for (size_t y=0; y< height; y++) {
		for (size_t x=0; x< width; x++) {
			destNums[y*height+x] = (*nums)[x*height+y];
		}
	}
	return transposedMatrix;
}

double Dmatrix::determinant() const {
	if (width != height) throw std::invalid_argument("matrix needs to be square.");
	if (width == 1) {
		return (*nums)[0].getNum();
	}
	if (width==2) {
		return ((*nums)[0].getNum() * (*nums)[3].getNum()) -
			   ( (*nums)[1].getNum() * (*nums)[2].getNum() );
	}
	double sum = 0.0;
	for (size_t i=0; i < width; i++) {
		sum += (i%2 == 0 ? 1.0 : -1.0) * (*nums)[i*width].getNum() * excludeRowAndCol(0, i).determinant();
	}
	return sum;
}

Dmatrix Dmatrix::excludeRowAndCol(uint32_t ex_col, uint32_t ex_row) const {
	Dmatrix excludedMatrix(
		(ex_col >= 0 && ex_col <= width) ? width-1 : width,
		(ex_row >= 0 && ex_row <= height) ? height-1 : height
	);

	Dnum* dest = excludedMatrix.getNums();

	for (size_t row = 0; row < height; row++) {
		if (row == ex_row) continue;
		for (size_t col = 0; col < height; col++) {
			if (col == ex_col) continue;
			(*dest) = (*nums)[row*width+col];
			dest++;
		}
	}

	return excludedMatrix;
}

Dmatrix Dmatrix::cofactor() const {
	Dmatrix cofactorMatrix(width, height);
	Dnum* destNums = cofactorMatrix.getNums();
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width ; x++) {
			destNums[y*height+x] =
				(x%2 == 0 ? 1.0 : -1.0) * (y%2 == 0 ? 1.0 : -1.0) *
				excludeRowAndCol(x,y).determinant();
		}
	}
	return cofactorMatrix;
}

Dmatrix Dmatrix::inverse() const {
	return cofactor().transpose().multiplyByFactor(1.0/determinant());
}

Dmatrix* Dmatrix::clone() const {
	return new Dmatrix(*this);
}

} // namespace torasu::tstd
