#ifndef CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_

#include <string>
#include <set>
#include <vector>

#include <torasu/torasu.hpp>

namespace torasu::tools {

inline void log_checked(torasu::LogInstruction li, torasu::LogLevel lvl, std::string msg) {
	if (li.level <= lvl) li.logger->log(lvl, msg);
}

class LogInfoRefBuilder {
private:
	std::set<LogId>* causes = nullptr;
	inline std::set<LogId>* getCauses() {
		if (causes == nullptr)
			causes = new std::set<LogId>();
		return causes;
	}

	inline LogId logCauseDirect(LogEntry* entry) {
		LogId tag = entry->addTag(linstr.logger);
		getCauses()->insert(tag);
		linstr.logger->log(entry);
		return tag;
	}
public:
	const torasu::LogInstruction& linstr;

	explicit LogInfoRefBuilder(const torasu::LogInstruction& linstr)
		: linstr(linstr) {}

	LogInfoRefBuilder(const torasu::LogInstruction& linstr, std::initializer_list<LogId> initList)
		: linstr(linstr) {
		for (LogId tag : initList) {
			addCause(tag);
		}
	}

	~LogInfoRefBuilder() {
		if (causes != nullptr) delete causes;
	}

	LogId logCause(LogEntry* entry) {
		bool doLog = linstr.level <= LogLevel::NO_LOGGING;
		if (LogMessage* lm = dynamic_cast<LogMessage*>(entry)) {
			if (linstr.level > lm->level) doLog = false;
		}
		if (doLog) {
			return logCauseDirect(entry);
		} else {
			delete entry;
			return LogId_MAX;
		}
	}

	LogId logCause(LogLevel level, std::string message) {
		LogInfoRef* typePun = nullptr;
		return logCause(level, message, typePun);
	}

	LogId logCause(LogLevel level, std::string message, LogInfoRef* subCauses) {
		if (linstr.level <= level) {
			return logCauseDirect(new LogMessage(level, message, subCauses));
		} else {
			return LogId_MAX;
		}
	}

	LogId logCause(LogLevel level, std::string message, LogInfoRefBuilder* subCauses) {
		return logCause(level, message, subCauses != nullptr ? subCauses->build() : nullptr);
	}

	LogId logCause(LogLevel level, std::string message, LogId causeTag) {
		LogInfoRefBuilder lirb(linstr);
		lirb.addCause(causeTag);
		return logCauseDirect(new LogMessage(level, message, lirb.build()));
	}

	void addCause(LogId tag) {
		if (tag != LogId_MAX) {
			getCauses()->insert(tag);
		}
	}

	void releaseCause(LogId tag) {
		if (causes != nullptr) {
			causes->erase(tag);
		}
	}

	torasu::LogInfoRef* build() {
		if (causes != nullptr && !causes->empty()) {
			auto* causes = new std::vector<std::vector<LogId>>();
			for (LogId tag : *this->causes) {
				causes->push_back({tag});
			}
			return new torasu::LogInfoRef(causes);
		} else {
			return nullptr;
		}
	}

};

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_LOG_TOOLS_HPP_
