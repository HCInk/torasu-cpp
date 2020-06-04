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
	std::string ident = std::string("STD::DBIMG");

	uint8_t* data;
	uint32_t width, height;

public:
	explicit Dbimg(Dbimg_FORMAT format);
	Dbimg(uint32_t width, uint32_t height);
	~Dbimg();

	std::string getIdent();
	DataDump* getData();

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

class Dbimg_FORMAT : public DataPackable {
private:
	const std::string formatIdent = std::string("STD::DBIMG");
	const std::string ident = std::string("STD::DBIMG_F");

	uint32_t width, height;

public:
	explicit Dbimg_FORMAT(std::string jsonStripped);
	explicit Dbimg_FORMAT(nlohmann::json jsonParsed);
	Dbimg_FORMAT(uint32_t width, uint32_t height);

	inline uint32_t getWidth() {
		return width;
	}

	inline uint32_t getHeight() {
		return height;
	}

	std::string getIdent();
	void load();
	nlohmann::json makeJson();

	inline ResultFormatSettings asFormat() {
		return ResultFormatSettings(formatIdent, NULL, this);
	}

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DBIMG_HPP_
