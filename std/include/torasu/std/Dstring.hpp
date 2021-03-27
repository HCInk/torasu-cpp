#ifndef STD_INCLUDE_TORASU_STD_DSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_HPP_

#include <string>

#include <torasu/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dstring : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DSTRING");

	std::string string;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	Dstring(std::string jsonStripped, bool json);
	Dstring(torasu::json jsonParsed, bool json);
	explicit Dstring(std::string str);

	const std::string& getString() const;

	torasu::DataResource::CompareResult compare(const DataResource* other) const override;
	std::string getIdent() const override;
	Dstring* clone() const override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_HPP_
