#ifndef STD_INCLUDE_TORASU_STD_DNUM_HPP_
#define STD_INCLUDE_TORASU_STD_DNUM_HPP_

#include <string>

#include <torasu/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dnum : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DNUM");

	double num;

public:
	explicit Dnum(std::string jsonStripped);
	explicit Dnum(torasu::json jsonParsed);
	explicit Dnum(double num);

	virtual ~Dnum();

	double getNum();

	std::string getIdent();
	void load();
	torasu::json makeJson();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DNUM_HPP_
