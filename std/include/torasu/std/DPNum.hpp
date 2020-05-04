#ifndef STD_INCLUDE_TORASU_STD_DPNUM_HPP_
#define STD_INCLUDE_TORASU_STD_DPNUM_HPP_

#include <string>

#include <nlohmann/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class DPNum : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DPNUM");

	double num;

public:
	explicit DPNum(std::string jsonStripped);
	explicit DPNum(nlohmann::json jsonParsed);
	explicit DPNum(double num);
	
	double getNum();

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DPNUM_HPP_
