#include "../include/torasu/std/Dfile.hpp"

#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

//
//	Dfile
//

Dfile::Dfile(uint64_t size) {
	this->data = new uint8_t[size];
	this->size = size;
}

Dfile::~Dfile() {
	delete[] data;
}

Dfile::Dfile(const Dfile& original)
	: Dfile(original.size) {
	std::copy(original.data, original.data+size, data);
}

std::string Dfile::getIdent() {
	return ident;
}

DataDump* Dfile::dumpResource() {
	return nullptr; // TODO Dfile-dumpResource
}

Dfile* Dfile::clone() {
	return new Dfile(*this);
}

//
//	FileBuilder
//

Dfile::FileBuilder::FileBuilder() {}
Dfile::FileBuilder::~FileBuilder() {
	clear();
}

void Dfile::FileBuilder::write(uint8_t* data, size_t dataSize) {
	if (size == pos) {
		uint8_t* newBuf = new uint8_t[dataSize];
		std::copy(data, data+dataSize, newBuf);
		buffers.insert( std::pair<size_t, std::pair<uint8_t*, size_t>>(pos, std::pair<uint8_t*, size_t>(newBuf, dataSize)));
		size += dataSize;
		// std::cout << " F:: APPEND " << dataSize << " @ " << pos << " (new size " << size << ")" << std::endl;
		pos = size;
	} else {
		auto found = buffers.upper_bound(pos);
		found--;
		while (dataSize > 0) {
			if (found != buffers.end()) {
				// std::cout << " F:: FOUND EXISTING @" << found->first << " (" << found->second.second << ") "
				// 	"- SEARCHED " << pos << " (" << dataSize << " pending)" << std::endl;
				uint8_t* buffer = found->second.first;
				size_t bufferOffset = pos - found->first;
				size_t bufferLeft = found->second.second-bufferOffset;
				size_t toWrite = std::min(dataSize, bufferLeft);
				// std::cout << " F:: .. WRITE O" << bufferOffset << " W" << toWrite << " (" << bufferLeft << ")" << std::endl;

				std::copy(data, data+toWrite, buffer+bufferOffset);

				data += toWrite;
				dataSize -= toWrite;
				pos += toWrite;
				found++;

			} else {
				write(data, dataSize);
				dataSize = 0;
			}
		}
	}
}

Dfile* Dfile::FileBuilder::compile() {
	auto* file = new torasu::tstd::Dfile(size);

	auto* data = file->getFileData();

	for (auto& buff : buffers) {
		std::copy(buff.second.first, buff.second.first+buff.second.second, data+buff.first);
	}

	return file;
}

void Dfile::FileBuilder::clear() {

	for (auto& buff : buffers) {
		delete[] buff.second.first;
	}
	buffers.clear();
	size = 0;
}

} // namespace torasu::tstd