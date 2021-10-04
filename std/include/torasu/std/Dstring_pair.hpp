#ifndef STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_

#include <string>

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
	explicit Dstring_pair(std::string jsonStripped);
	explicit Dstring_pair(torasu::json jsonParsed);

	Dstring_pair(std::string a, std::string b);
	~Dstring_pair();

	torasu::Identifier getType() const override;
	Dstring_pair* clone() const override;

	const std::string& getA();
	const std::string& getB();
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_PAIR_HPP_
