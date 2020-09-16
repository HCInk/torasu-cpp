#include "../include/torasu/std/Dfile.hpp"

#include <iostream>
#include <torasu/json.hpp>

using namespace std;
using json = torasu::json;

namespace torasu::tstd {

Dfile::Dfile(uint64_t size) {
	this->data = new uint8_t[size];
	this->size = size;
}

Dfile::~Dfile() {
	delete[] data;
}

std::string Dfile::getIdent() {
	return ident;
}

DataDump* Dfile::dumpResource() {
	return nullptr; // TODO Dfile-dumpResource
}

} // namespace torasu::tstd