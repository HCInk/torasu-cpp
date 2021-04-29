#ifndef CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_
#define CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_

#include <string>

#include <torasu/DataPackable.hpp>

namespace torasu {

class Dlog_entry : public torasu::DataPackable {
private:
	LogEntry* entry = nullptr;

public:
	explicit Dlog_entry(const Dlog_entry& original);
	explicit Dlog_entry(const LogEntry& entry);
	explicit Dlog_entry(torasu::json json);
	explicit Dlog_entry(std::string jsonStr);
	~Dlog_entry();
	LogEntry getEntry();

	std::string getIdent() const override;
	void load() override;
	torasu::json makeJson() override;

	Dlog_entry* clone() const override;

};

} // namespace torasu

#endif // CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_
