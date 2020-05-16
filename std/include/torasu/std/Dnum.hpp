#ifndef STD_INCLUDE_TORASU_STD_DNUM_HPP_
#define STD_INCLUDE_TORASU_STD_DNUM_HPP_

#include <string>

#include <nlohmann/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dnum : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DPNUM");

	double num;

public:
	explicit Dnum(std::string jsonStripped);
	explicit Dnum(nlohmann::json jsonParsed);
	explicit Dnum(double num);

	double getNum();

	virtual std::string getIdent();
	virtual void load();
	virtual nlohmann::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DNUM_HPP_
