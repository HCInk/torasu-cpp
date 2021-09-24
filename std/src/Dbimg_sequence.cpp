#include "../include/torasu/std/Dbimg_sequence.hpp"

#include <torasu/std/Dbimg.hpp>

namespace torasu::tstd {

Dbimg_sequence::Dbimg_sequence() {}

Dbimg_sequence::~Dbimg_sequence() {
	for (auto& frame : frames) {
		delete frame.second;
	}
}

Dbimg_sequence::Dbimg_sequence(const Dbimg_sequence& original)
	: time_padding(original.time_padding) {

	for (auto& frame : original.frames) {
		frames.insert(std::make_pair(frame.first, frame.second->clone()));
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

torasu::Identifier Dbimg_sequence::getType() const {
	return "STD::DBIMG_SEQUENCE";
}

DataDump* Dbimg_sequence::dumpResource() {
	return nullptr; // TODO DataDump of Dbimg_sequence
}

Dbimg_sequence* Dbimg_sequence::clone() const {
	return new Dbimg_sequence(*this);
}

} // namespace torasu::tstd