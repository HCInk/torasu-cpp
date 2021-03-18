#ifndef STD_INCLUDE_TORASU_STD_DFILE_HPP_
#define STD_INCLUDE_TORASU_STD_DFILE_HPP_

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <utility>

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
	Dfile(const Dfile&);

	std::string getIdent() const override;
	DataDump* dumpResource() override;
	Dfile* clone() const override;

	inline uint8_t* getFileData() {
		return data;
	}

	inline uint64_t getFileSize() {
		return size;
	}

	class FileBuilder {
	private:
		size_t size = 0;
		std::map<size_t, std::pair<uint8_t*, size_t>, std::less<size_t>> buffers;

	public:
		size_t pos = 0;

		FileBuilder();
		~FileBuilder();

		void write(uint8_t* data, size_t dataSize);
		Dfile* compile();
		void clear();
	};
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DFILE_HPP_
