#include "../include/torasu/std/Rlocal_file.hpp"

#include <string>
#include <fstream>
#include <iostream>

#include <torasu/torasu.hpp>
#include <torasu/log_tools.hpp>
#include <torasu/std/Dfile.hpp>

using namespace std;

using namespace torasu::tools;

namespace torasu::tstd {

Rlocal_file::Rlocal_file(string path) : SimpleRenderable("STD::RLOCAL_FILE", true, false) {
	this->path = path;
}

Rlocal_file::~Rlocal_file() {

}

ResultSegment* Rlocal_file::renderSegment(ResultSegmentSettings* resSettings, RenderInstruction* ri) {

	if (resSettings->getPipeline().compare(pipeline) == 0) {

		ifstream ifs(path, ios::binary|ios::ate);
		ifstream::pos_type pos = ifs.tellg();

		if (pos == ifstream::pos_type(-1)) {
			throw std::runtime_error("Failed to read file \"" + path + "\"");
		}

		uint64_t size = pos;
		Dfile* dfile = new Dfile(size);

		char* pChars = reinterpret_cast<char*>(dfile->getFileData());
		ifs.seekg(0, ios::beg);
		ifs.read(pChars, size);

		torasu::tools::log_checked(ri->getLogInstruction(), LogLevel::DEBUG,
								   "Loaded local-file \"" + path + "\" (" + std::to_string(dfile->getFileSize()) + "byte)");

		return new ResultSegment(ResultSegmentStatus_OK, dfile, true);

	} else {
		return new ResultSegment(ResultSegmentStatus_INVALID_SEGMENT);
	}
}

DataResource* Rlocal_file::getData() {
	return NULL; // TODO Rlocal_file getData
}

void Rlocal_file::setData(DataResource* data) {
	// TODO Rlocal_file setData
}

} // namespace torasu::tstd