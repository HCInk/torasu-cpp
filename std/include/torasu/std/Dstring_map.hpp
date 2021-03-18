#ifndef STD_INCLUDE_TORASU_STD_DSTRING_MAP_HPP_
#define STD_INCLUDE_TORASU_STD_DSTRING_MAP_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Dstring_map : public torasu::DataPackable {
private:
	std::map<std::string, std::string> map;

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	explicit Dstring_map(std::string jsonStripped);
	explicit Dstring_map(torasu::json jsonParsed);

	Dstring_map();
	~Dstring_map();

	const std::map<std::string, std::string>& getMap() const;
	const std::string* get(const std::string& key) const;
	void set(const std::string& key, const std::string& value);
	void erase(const std::string& key);

	std::string getIdent() const override;
	Dstring_map* clone() const override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_DSTRING_MAP_HPP_
