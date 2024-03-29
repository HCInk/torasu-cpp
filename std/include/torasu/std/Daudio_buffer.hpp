//
// Created by Yann Holme Nielsen on 13/06/2020.
//

#ifndef STD_INCLUDE_TORASU_STD_DAUDIO_BUFFER_HPP_
#define STD_INCLUDE_TORASU_STD_DAUDIO_BUFFER_HPP_

#include <string>
#include <cstdint>
#include <cstddef>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

/**Format of channel
 */
enum Daudio_buffer_CHFMT {
	UNKNOWN = -1,
	FLOAT32,
	FLOAT64
};

/**Single audio-channel struct
 */
struct Daudio_buffer_CHANNEL {

	uint8_t* data;
	size_t dataSize;
	size_t sampleRate;
	size_t sampleSize;
	Daudio_buffer_CHFMT format;

};

/**Format for Daudio_buffer
 * @see Daudio_buffer
 */
class Daudio_buffer_FORMAT : public ResultFormatSettings, public DataPackable {

private:
	int bitrate;
	Daudio_buffer_CHFMT format;

public:

	Daudio_buffer_FORMAT(int bitrate, Daudio_buffer_CHFMT format);

	explicit Daudio_buffer_FORMAT(const torasu::json& initialJson);
	explicit Daudio_buffer_FORMAT(const std::string& initialSerializedJson);

	inline int getBitrate() const  {
		return bitrate;
	}

	inline Daudio_buffer_CHFMT getFormat() const {
		return format;
	}

	void load() override;
	torasu::json makeJson() override;
	Daudio_buffer_FORMAT* clone() const override;
};


/**TORASU's standard for holding raw audio
 */
class Daudio_buffer : public DataResource {
private:
	Daudio_buffer_CHANNEL* channels;
	size_t channelCount;

public:
	explicit Daudio_buffer(size_t channelCount);
	explicit Daudio_buffer(size_t channelCount, size_t sampleRate, Daudio_buffer_CHFMT format, size_t sampleSize, size_t dataSize);
	~Daudio_buffer();
	Daudio_buffer(const Daudio_buffer&);

	uint8_t* initChannel(size_t channelIndex, size_t sampleRate, Daudio_buffer_CHFMT format, size_t sampleSize, size_t dataSize, bool fromInit = false);
	Daudio_buffer_CHANNEL* getChannels() const;
	size_t getChannelCount() const;

	Identifier getType() const override;
	DataDump* dumpResource() override;
	Daudio_buffer* clone() const override;

};

} // namespace torasu::tstd

#endif //STD_INCLUDE_TORASU_STD_DAUDIO_BUFFER_HPP_
