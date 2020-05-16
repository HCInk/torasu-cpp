#include "../include/torasu/std/Dfile.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

namespace torasu::tstd {

Dfile::Dfile(uint64_t size) {
	this->data = new vector<uint8_t>(size);
}

Dfile::~Dfile() {
	delete data;
}

std::string Dfile::getIdent() {
	return ident;
}

DataDump* Dfile::getData() {
	return NULL; // TODO Dfile-getData
}

std::vector<uint8_t>* Dfile::getFileData() {
	return data;
}

} // namespace torasu::tstd