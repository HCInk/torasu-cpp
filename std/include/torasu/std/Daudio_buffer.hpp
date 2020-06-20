//
// Created by Yann Holme Nielsen on 13/06/2020.
//

#ifndef STD_INCLUDE_TORASU_DAUDIO_BUFFER_HPP
#define STD_INCLUDE_TORASU_DAUDIO_BUFFER_HPP

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
	Daudio_buffer_CHFMT format;

};

/**Format for Daudio_buffer
 * @see Daudio_buffer
 */
class Daudio_buffer_FORMAT : public DataPackable {

private:
	const std::string dataIdent = std::string("STD::DAUDIO_BUFFER");
	const std::string formatIdent = std::string("STD::DAUDIO_BUFFER_F");

	int bitrate;
	Daudio_buffer_CHFMT format;

public:

	Daudio_buffer_FORMAT(int bitrate, Daudio_buffer_CHFMT format);

	Daudio_buffer_FORMAT(const nlohmann::json &initialJson);
	Daudio_buffer_FORMAT(const std::string &initialSerializedJson);

	int getBitrate() const;
	Daudio_buffer_CHFMT getFormat() const;

	void setBitrate(int bitrate);
	void setFormat(Daudio_buffer_CHFMT format);

	std::string getIdent() override;

	void load() override;
	nlohmann::json makeJson() override;

	inline ResultFormatSettings asFormat() {
		return ResultFormatSettings(formatIdent, NULL, this);
	}

};


/**TORASU's standard for holding raw audio
 */
class Daudio_buffer : public DataResource {
private:

	Daudio_buffer_CHANNEL* channels;
	size_t channelCount;

public:

	Daudio_buffer(size_t channelCount);
	Daudio_buffer(size_t channelCount, size_t sampleRate, Daudio_buffer_CHFMT format, size_t dataSize);
	virtual ~Daudio_buffer();

	uint8_t* initChannel(size_t channelIndex, size_t sampleRate, Daudio_buffer_CHFMT format, size_t dataSize, bool fromInit = false);
	Daudio_buffer_CHANNEL* getChannels() const;
	size_t getChannelCount() const;

	DataDump* getData() override;
	std::string getIdent() override;

};

} // namespace torasu::tstd

#endif //TORASU_CPP_EXAMPLES_DAUDIO_BUFFER_HPP
