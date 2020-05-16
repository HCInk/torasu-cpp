#ifndef STD_INCLUDE_TORASU_STD_DRBIMG_HPP_
#define STD_INCLUDE_TORASU_STD_DRBIMG_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dbimg;
class DRBImg_FORMAT;

class Dbimg : public DataResource {
private:
	std::string ident = std::string("STD::DRBIMG");

	uint8_t* data;
	uint32_t width, height;
	uint64_t bufferSize;

public:
	explicit Dbimg(DRBImg_FORMAT format);
	Dbimg(uint32_t width, uint32_t height);
	virtual ~Dbimg();

	virtual std::string getIdent();
	virtual DataDump* getData();

	uint32_t getWidth();
	uint32_t getHeight();
	uint64_t getBufferSize();
	unsigned char* getImageData();


};

class DRBImg_FORMAT : public DataPackable {
private:
	const std::string formatIdent = std::string("STD::DRBIMG");
	const std::string ident = std::string("STD::DPF_DRBIMG");

	u_int32_t width, height;

public:
	explicit DRBImg_FORMAT(std::string jsonStripped);
	explicit DRBImg_FORMAT(nlohmann::json jsonParsed);
	DRBImg_FORMAT(u_int32_t width, u_int32_t height);

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

#endif // STD_INCLUDE_TORASU_STD_DRBIMG_HPP_
