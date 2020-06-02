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

	std::vector<uint8_t>* data;

public:
	explicit Dfile(std::vector<uint8_t>* data);
	explicit Dfile(uint64_t size);
	~Dfile();

	std::string getIdent();
	DataDump* getData();

	std::vector<uint8_t>* getFileData();

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DFILE_HPP_
