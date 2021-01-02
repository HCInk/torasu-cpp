#ifndef CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_
#define CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_

#include <string>

#include <torasu/DataPackable.hpp>

namespace torasu {

class Dlog_entry : public torasu::DataPackable {
private:
    LogEntry* entry = nullptr;

public:
    Dlog_entry(const LogEntry& entry);
    Dlog_entry(torasu::json json);
    Dlog_entry(std::string jsonStr);
    ~Dlog_entry();
    LogEntry getEntry();

    std::string getIdent() override;
    void load() override;
    torasu::json makeJson() override;

    Dlog_entry* clone() override;

};

} // namespace torasu

#endif // CORE_INCLUDE_TORASU_DLOG_ENTRY_HPP_
