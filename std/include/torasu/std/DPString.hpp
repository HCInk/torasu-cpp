#ifndef STD_INCLUDE_TORASU_STD_DPSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_DPSTRING_HPP_

#include <string>

#include <nlohmann/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class DPString : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DPSTRING");

	std::string string;

public:
	DPString(std::string jsonStripped, bool json);
	DPString(nlohmann::json jsonParsed, bool json);
	explicit DPString(std::string str);

	std::string getString();

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DPSTRING_HPP_
