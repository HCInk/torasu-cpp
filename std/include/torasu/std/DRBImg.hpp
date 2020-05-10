#ifndef STD_INCLUDE_TORASU_STD_DRBIMG_HPP_
#define STD_INCLUDE_TORASU_STD_DRBIMG_HPP_

#include <string>

#include <torasu/torasu.hpp>

namespace torasu::tstd {

class DRBImg : public DataResource {
private:
	std::string ident = std::string("STD::DRBIMG");

	uint8_t* data;
	uint32_t width, height;

public:
	DRBImg(uint32_t width, uint32_t height);
	virtual ~DRBImg();

	virtual std::string getIdent();
	virtual DataDump* getData();

	uint32_t getWidth();
	uint32_t getHeight();
	unsigned char* getImageData();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DRBIMG_HPP_
