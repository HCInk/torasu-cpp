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
public:
	/** @brief  CropInfo describes how the canvas has been cut or extended to each side */
	struct CropInfo {
		/** @breif crop in px at left side, postive for pixels removed, negative fox pixels expanded */
		int32_t left = 0;
		/** @breif crop in px at right side, postive for pixels removed, negative fox pixels expanded */
		int32_t right = 0;
		/** @breif crop in px at top, postive for pixels removed, negative fox pixels expanded */
		int32_t top = 0;
		/** @breif crop in px at bottom, postive for pixels removed, negative fox pixels expanded */
		int32_t bottom = 0;
	};
private:
	uint8_t* data;
	/** @brief resolution of image data (including modifications of the crop) */
	uint32_t width, height;
	/** @brief CropInfo, contains current cropping state of the image and that other cropping-modes are available
	 * 			- if not provided it will mean that no other cropping-modes are available */
	CropInfo* cropInfo = nullptr;

public:
	explicit Dbimg(Dbimg_FORMAT format);
	Dbimg(uint32_t width, uint32_t height, Dbimg::CropInfo* cropInfo = nullptr);
	Dbimg(const Dbimg& copy);
	~Dbimg();

	void clear();

	Identifier getType() const override;
	DataDump* dumpResource() override;
	Dbimg* clone() const override;

	inline uint32_t getWidth() const {
		return width;
	}

	inline uint32_t getHeight() const {
		return height;
	}

	inline const Dbimg::CropInfo* getCropInfo() const {
		return cropInfo;
	}

	inline uint8_t* getImageData() const {
		return data;
	}
};

class Dbimg_FORMAT : public ResultFormatSettings, public DataPackable {
private:
	/** @brief Resultion of image-core (excluding any modifications of crops) */
	uint32_t width, height;
	/** @brief CropInfo, if provided it indicates that cropping is available
	 * 			- the contained data indicates that crop to be required,
	 * 				if the content is smaller the resulting crop may be tightened to leave out parts where no content is*/
	Dbimg::CropInfo* cropInfo = nullptr;

public:
	Dbimg_FORMAT(uint32_t width, uint32_t height, Dbimg::CropInfo* cropInfo = nullptr);
	explicit Dbimg_FORMAT(const torasu::json& jsonParsed);
	explicit Dbimg_FORMAT(const std::string& jsonStripped);
	~Dbimg_FORMAT();

	inline uint32_t getWidth() const {
		return width;
	}

	inline uint32_t getHeight() const {
		return height;
	}

	inline const Dbimg::CropInfo* getCropInfo() const {
		return cropInfo;
	}

	void load() override;
	torasu::json makeJson() override;
	Dbimg_FORMAT* clone() const override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DBIMG_HPP_
