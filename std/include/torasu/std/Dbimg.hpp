#ifndef STD_INCLUDE_TORASU_STD_DBIMG_HPP_
#define STD_INCLUDE_TORASU_STD_DBIMG_HPP_

#include <string>
#include <vector>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dbimg;
class Dbimg_FORMAT;

class Dbimg : public DataResource {
private:
	uint8_t* data;
	uint32_t width, height;

public:
	explicit Dbimg(Dbimg_FORMAT format);
	Dbimg(uint32_t width, uint32_t height);
	Dbimg(const Dbimg& copy);
	~Dbimg();

	std::string getIdent() override;
	DataDump* dumpResource() override;
	Dbimg* clone() override;

	inline uint32_t getWidth() {
		return width;
	}

	inline uint32_t getHeight() {
		return height;
	}

	inline uint8_t* getImageData() {
		return data;
	}
};

class Dbimg_FORMAT : public ResultFormatSettings, public DataPackable {
private:
	uint32_t width, height;

public:
	Dbimg_FORMAT(uint32_t width, uint32_t height);
	explicit Dbimg_FORMAT(const torasu::json& jsonParsed);
	explicit Dbimg_FORMAT(const std::string& jsonStripped);

	inline uint32_t getWidth() {
		return width;
	}

	inline uint32_t getHeight() {
		return height;
	}

	void load() override;
	torasu::json makeJson() override;
	Dbimg_FORMAT* clone() override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DBIMG_HPP_
