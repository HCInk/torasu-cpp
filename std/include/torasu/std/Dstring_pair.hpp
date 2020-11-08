#ifndef STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dstring_pair : public torasu::DataPackable {
private:
	std::string a;
	std::string b;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	Dstring_pair(std::string jsonStripped);
	Dstring_pair(torasu::json jsonParsed);

	Dstring_pair(std::string a, std::string b);
	~Dstring_pair();

	std::string getIdent() override;
	
	std::string getA();
	std::string getB();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_
