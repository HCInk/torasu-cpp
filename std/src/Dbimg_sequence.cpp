#include "../include/torasu/std/Dbimg_sequence.hpp"

#include <torasu/std/Dbimg.hpp>

namespace torasu::tstd {

Dbimg_sequence::Dbimg_sequence() {}

Dbimg_sequence::~Dbimg_sequence() {
	for (auto& frame : frames) {
		delete frame.second;
	}
}

Dbimg* Dbimg_sequence::addFrame(double pts, Dbimg_FORMAT format) {
	Dbimg* bimg = new Dbimg(format);
	frames.insert(std::make_pair(pts, bimg));
	return bimg;
}

std::multimap<double, torasu::tstd::Dbimg*, std::less<double>>& Dbimg_sequence::getFrames() {
	return frames;
}

std::string Dbimg_sequence::getIdent() {
	return "STD::DBIMG_SEQUENCE";
}

DataDump* Dbimg_sequence::dumpResource() {
	return nullptr; // TODO DataDump of Dbimg_sequence
}

} // namespace torasu::tstd