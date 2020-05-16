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

	std::vector<uint8_t>* data;
	uint32_t width, height;

public:
	explicit Dbimg(Dbimg_FORMAT format);
	Dbimg(uint32_t width, uint32_t height);
	Dbimg(uint32_t width, uint32_t height, std::vector<uint8_t>* data);
	virtual ~Dbimg();

	virtual std::string getIdent();
	virtual DataDump* getData();

	uint32_t getWidth();
	uint32_t getHeight();
	std::vector<uint8_t>* getImageData();
	uint8_t* getDataAddress();
	std::size_t getBufferSize();


};

class Dbimg_FORMAT : public DataPackable {
private:
	const std::string formatIdent = std::string("STD::DBIMG");
	const std::string ident = std::string("STD::DBIMG_F");

	u_int32_t width, height;

public:
	explicit Dbimg_FORMAT(std::string jsonStripped);
	explicit Dbimg_FORMAT(nlohmann::json jsonParsed);
	Dbimg_FORMAT(u_int32_t width, u_int32_t height);

	u_int32_t getWidth();
	u_int32_t getHeight();

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();

	inline ResultFormatSettings asFormat() {
		return ResultFormatSettings(formatIdent, NULL, this);
	}
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DBIMG_HPP_
