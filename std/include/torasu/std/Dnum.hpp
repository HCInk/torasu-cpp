#ifndef STD_INCLUDE_TORASU_STD_DNUM_HPP_
#define STD_INCLUDE_TORASU_STD_DNUM_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/slot_tools.hpp>

namespace torasu::tstd {

class Dnum : public torasu::DataPackable {

private:
	const std::string ident = std::string("STD::DNUM");

	double num;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	explicit Dnum(std::string jsonStripped);
	explicit Dnum(torasu::json jsonParsed);
	explicit Dnum(double num);

	virtual ~Dnum();

	double getNum();

	std::string getIdent() override;
	Dnum* clone() override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DNUM_HPP_
