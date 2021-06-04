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
	Dnum();
	/* implicit */ Dnum(double num);
	explicit Dnum(std::string jsonStripped);
	explicit Dnum(torasu::json jsonParsed);

	virtual ~Dnum();

	void operator=(Dnum value);
	void operator=(double value);

	double getNum();

	std::string getIdent() const override;
	Dnum* clone() const override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DNUM_HPP_
