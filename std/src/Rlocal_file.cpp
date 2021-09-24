#include "../include/torasu/std/Rlocal_file.hpp"

#include <string>
#include <fstream>

#include <torasu/torasu.hpp>
#include <torasu/log_tools.hpp>
#include <torasu/std/Dfile.hpp>

using namespace std;

using namespace torasu::tools;

namespace torasu::tstd {

Rlocal_file::Rlocal_file(string path) : SimpleRenderable(true, false) {
	this->path = path;
}

Rlocal_file::~Rlocal_file() {}

Identifier Rlocal_file::getType() {
	return "STD::RLOCAL_FILE";
}

torasu::ResultSegment* Rlocal_file::render(torasu::RenderInstruction* ri) {
	if (ri->getResultSettings()->getPipeline() == TORASU_STD_PL_FILE) {

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

torasu::DataResource* Rlocal_file::getData() {
	return nullptr; // TODO Rlocal_file getData
}

void Rlocal_file::setData(torasu::DataResource* data) {
	// TODO Rlocal_file setData
}

} // namespace torasu::tstd