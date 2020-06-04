#ifndef STD_INCLUDE_TORASU_STD_DFILE_HPP_
#define STD_INCLUDE_TORASU_STD_DFILE_HPP_

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dfile : public DataResource {
private:
	std::string ident = std::string("STD::DFILE");

	uint8_t* data;
	uint64_t size;

public:
	explicit Dfile(uint64_t size);
	~Dfile();

	std::string getIdent();
	DataDump* getData();

	inline uint8_t* getFileData() {
		return data;
	}

	inline uint64_t getFileSize() {
		return size;
	}
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DFILE_HPP_
