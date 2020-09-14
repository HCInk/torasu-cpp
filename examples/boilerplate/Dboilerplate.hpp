#ifndef EXAMPLES_BOILERPLATE_DBOILERPLATE_HPP_
#define EXAMPLES_BOILERPLATE_DBOILERPLATE_HPP_

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::texample {

class Dboilerplate : public torasu::DataPackable {
private:
	std::string str;
	double num;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	Dboilerplate(std::string str, double num);
	~Dboilerplate();

	std::string getIdent() override;
	std::string getStr();
	double getNum();
};

} // namespace torasu::texample

#endif // EXAMPLES_BOILERPLATE_DBOILERPLATE_HPP_
