#ifndef STD_INCLUDE_TORASU_STD_DSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_HPP_

#include <string>

#include <nlohmann/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dstring : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DPSTRING");

	std::string string;

public:
	Dstring(std::string jsonStripped, bool json);
	Dstring(nlohmann::json jsonParsed, bool json);
	explicit Dstring(std::string str);

	std::string getString();

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_HPP_
