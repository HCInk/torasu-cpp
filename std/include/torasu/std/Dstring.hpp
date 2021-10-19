#ifndef STD_INCLUDE_TORASU_STD_DSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_HPP_

#include <string>

#include <torasu/json.hpp>
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dstring : public torasu::DataPackable {

private:
	std::string string;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	Dstring(std::string jsonStripped, bool json);
	Dstring(torasu::json jsonParsed, bool json);
	explicit Dstring(std::string str);

	const std::string& getString() const;

	torasu::DataResource::CompareResult compare(const DataResource* other) const override;
	torasu::Identifier getType() const override;
	Dstring* clone() const override;

	static const torasu::DataPackableFactory* const FACTORY;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_HPP_
