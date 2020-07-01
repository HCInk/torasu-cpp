//
// Created by Yann Holme Nielsen on 13/06/2020.
//

#include "../include/torasu/std/Daudio_buffer.hpp"

namespace torasu::tstd {

Daudio_buffer::Daudio_buffer(size_t channelCount) : channelCount(channelCount) {
	channels = new Daudio_buffer_CHANNEL[channelCount];
	for (size_t i = 0; i < channelCount; ++i) {
		initChannel(i, 0, UNKNOWN, 0, true);
	}
}

Daudio_buffer::Daudio_buffer(size_t channelCount, size_t sampleRate, Daudio_buffer_CHFMT format, size_t sampleSize, size_t dataSize) : channelCount(channelCount) {
	channels = new Daudio_buffer_CHANNEL[channelCount];
	for (size_t i = 0; i < channelCount; ++i) {
		initChannel(i, sampleRate, format, dataSize, sampleSize, true);
	}
}

Daudio_buffer::~Daudio_buffer() {
	for (size_t i = 0; i < channelCount; ++i) {
		delete[] channels[i].data;
	}
	delete[] channels;
}

Daudio_buffer_CHANNEL* Daudio_buffer::getChannels() const {
	return channels;
}

size_t Daudio_buffer::getChannelCount() const {
	return channelCount;
}

DataDump *Daudio_buffer::getData() {
	throw std::logic_error("getData() not implemented for Daudio_buffer");
}

std::string Daudio_buffer::getIdent() {
	return "STD::DAUDIO_BUFFER";
}
//                                                                                                                         Only delete old ref if not from instantiation
uint8_t* Daudio_buffer::initChannel(size_t channelIndex, size_t sampleRate, Daudio_buffer_CHFMT format, size_t dataSize,size_t sampleSize, bool fromInit) {

	if (channelIndex < 0 || channelIndex >= channelCount) {
		throw std::out_of_range(std::string("Channel index out of bounds: ") + std::to_string(channelIndex)
								+ " (allowed range: 0-" + std::to_string(channelCount-1));
	}
	if(!fromInit)delete[] channels[channelIndex].data;

	uint8_t* data = new uint8_t[dataSize];

	channels[channelIndex] = (Daudio_buffer_CHANNEL) {
		.data = data,
		.dataSize = dataSize,
		.sampleRate = sampleRate,
		.sampleSize = sampleSize,
		.format = format
	};

	return data;
}

Daudio_buffer_FORMAT::Daudio_buffer_FORMAT(int bitrate, Daudio_buffer_CHFMT format) : bitrate(bitrate),
	format(format) {}

Daudio_buffer_FORMAT::Daudio_buffer_FORMAT(const nlohmann::json &initialJson) : DataPackable(initialJson) {}
Daudio_buffer_FORMAT::Daudio_buffer_FORMAT(const std::string &initialSerializedJson) : DataPackable(
		initialSerializedJson) {}

int Daudio_buffer_FORMAT::getBitrate() const {
	return bitrate;
}

Daudio_buffer_CHFMT Daudio_buffer_FORMAT::getFormat() const {
	return format;
}

void Daudio_buffer_FORMAT::setBitrate(int bitrate) {
	Daudio_buffer_FORMAT::bitrate = bitrate;
}

void Daudio_buffer_FORMAT::setFormat(Daudio_buffer_CHFMT format) {
	Daudio_buffer_FORMAT::format = format;
}

std::string Daudio_buffer_FORMAT::getIdent() {
	return formatIdent;
}

void Daudio_buffer_FORMAT::load() {
	auto json = getJson();

	if (json["rate"].is_number()) {
		bitrate = json["rate"];
	} else {
		bitrate = 0;
	}

	if (json["fmt"].is_number()) {
		format = json["fmt"];
	} else {
		format = UNKNOWN;
	}

}

nlohmann::json Daudio_buffer_FORMAT::makeJson() {
	return {
		{"rate", bitrate},
		{"fmt", format},
	};
}


} // namespace torasu::tstd
