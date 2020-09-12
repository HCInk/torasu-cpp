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

public:
	Dstring(std::string jsonStripped, bool json);
	Dstring(torasu::json jsonParsed, bool json);
	explicit Dstring(std::string str);

	std::string getString();

	virtual std::string getIdent();
	virtual void load();
	virtual torasu::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_HPP_
