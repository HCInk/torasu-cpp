#include "../include/torasu/std/LIcore_logger.hpp"

#include <memory>
#include <iostream>

namespace {

const char* getLvLName(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return "TRACE";
	case torasu::LogLevel::DEBUG:
		return "DEBUG";
	case torasu::LogLevel::INFO:
		return "INFO ";
	case torasu::LogLevel::WARN:
		return "WARN ";
	case torasu::LogLevel::ERROR:
		return "ERROR";
	case torasu::LogLevel::SERVERE_ERROR:
		return "S-ERR";
	default:
		return "UNKWN";
	}
}

static const char* ANSI_RESET = "\33[0m";
static const char* ANSI_HIGHLIGTED_RED = "\33[97m\33[101m";
static const char* ANSI_RED = "\33[91m";
static const char* ANSI_YELLOW = "\33[93m";
static const char* ANSI_BLUE = "\33[94m";
static const char* ANSI_DARK_GREEN = "\33[32m";
static const char* ANSI_GRAY = "\33[90m";
static const char* ANSI_CYAN = "\33[36m";

const char* getLvlAnsi(torasu::LogLevel lvl) {
	switch (lvl) {
	case torasu::LogLevel::TRACE:
		return ANSI_GRAY;
	case torasu::LogLevel::DEBUG:
		return ANSI_DARK_GREEN;
	case torasu::LogLevel::INFO:
		return ANSI_BLUE;
	case torasu::LogLevel::WARN:
		return ANSI_YELLOW;
	case torasu::LogLevel::ERROR:
		return ANSI_RED;
	case torasu::LogLevel::SERVERE_ERROR:
		return ANSI_HIGHLIGTED_RED;
	default:
		return "";
	}
}

static std::string groupStackToStr(std::vector<torasu::LogId> groupStack, bool tag=false) {
	std::string str;
	size_t pathSize = groupStack.size();
	const torasu::LogId* gsit = groupStack.data() + pathSize - 1;
	if (tag) pathSize--;
	for (size_t i = pathSize; i > 0; i--) {
		str += "/" + std::to_string(*gsit);
		gsit--;
	}

	if (tag) str += " [" + std::to_string(*gsit) + "]";

	return str;
}

} // namespace


namespace torasu::tstd {

LIcore_logger::LIcore_logger() {}

LIcore_logger::LIcore_logger(bool useAnsi) : useAnsi(useAnsi) {}

void LIcore_logger::log(LogEntry* entry) {
	std::unique_lock lck(logMutex);
	std::unique_ptr<LogEntry> entryHolder(entry);

	std::string message;

	switch (entry->type) {
	case LT_GROUP_START: {
			auto* startEntry = static_cast<LogGroupStart*>(entry);
			if (useAnsi) message += ANSI_GRAY;
			message += "====  " + groupStackToStr(entry->groupStack) + " * " + startEntry->name;
			logstore.create(entry->groupStack, startEntry->name);
		}
		break;
	case LT_GROUP_END:
		break;
	case LT_GROUP_UNREF: {
			// message += "(FREE)" + groupStackToStr(entry->groupStack) + " * " + entry->text;
			std::unique_ptr<std::stack<LIcore_logger_logstore::StoreGroup*>> resolveStack(logstore.resolve(entry->groupStack));
			auto found = resolveStack->top();
			if (found == nullptr) throw std::runtime_error("Failed to unregister log-group: Group not found!");
			logstore.remove(found);
		}
		break;

	default: {
			// Prefix

			if (entry->type == LT_MESSAGE) {
				auto* msgEntry = static_cast<LogMessage*>(entry);
				if (useAnsi) message += getLvlAnsi(msgEntry->level);
				message += getLvLName(msgEntry->level);
			} else {
				if (useAnsi) message += ANSI_CYAN;
				message += "DATA ";
			}

			message += " ";

			// Group Display

			if (!entry->groupStack.empty()) {

				bool isTag;
				std::unique_ptr<std::stack<LIcore_logger_logstore::StoreGroup*>> resolveStack(logstore.resolve(entry->groupStack, &isTag));

				message += groupStackToStr(entry->groupStack, isTag) + " \t";

				LIcore_logger_logstore::StoreGroup* foundGroup = resolveStack->top();
				if (foundGroup != nullptr) {
					if (isTag) foundGroup->tagged[entry->groupStack[0]] = entryHolder.get();
					foundGroup->logs.push_back(entryHolder.release());
					message += foundGroup->name;
				} else {
					message += "(UNKNOWN)";
				}
				message += ": ";
			} else {
				logstore.getRoot()->logs.push_back(entryHolder.release());
			}


			// Message

			if (entry->type == LT_MESSAGE) {
				message += static_cast<LogMessage*>(entry)->text;
			} else {
				message += "[DataPacket-" + std::to_string(entry->type) + "]";
			}


		}
		break;
	}

	if (!message.empty()) {
		// Suffix

		if (useAnsi) {
			message += ANSI_RESET;
		}

		// Print

		std::cout << message << std::endl;
	}
}

torasu::LogId LIcore_logger::fetchSubId() {

	std::unique_lock lock(subIdCounterLock);
	auto subId = subIdCounter;
	subIdCounter++;
	return subId;

}

std::vector<LogId>* LIcore_logger::pathFromParent	(LogInterface* parent) const {
	if (parent == this) return new std::vector<LogId>(); // Found: parent is this
	else return nullptr; // Not found
}

torasu::tstd::LIcore_logger_logstore::LIcore_logger_logstore() {}
torasu::tstd::LIcore_logger_logstore::~LIcore_logger_logstore() {}

torasu::tstd::LIcore_logger_logstore::StoreGroup::StoreGroup() {}

torasu::tstd::LIcore_logger_logstore::StoreGroup::StoreGroup(std::string name, torasu::LogId logId, StoreGroup* parent)
	: owner(parent), name(name) {

	parent->sub[logId] = this;
	owner->cleanupList.insert(this);

}

torasu::tstd::LIcore_logger_logstore::StoreGroup::~StoreGroup() {
	// std::cout << " RM " << this << std::endl;
	for (StoreGroup* toClean : cleanupList) delete toClean;
	for (LogEntry* entry : logs) delete entry;
}

std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>* torasu::tstd::LIcore_logger_logstore::resolve(const std::vector<LogId>& path, bool* tag, size_t pathDepth) {
	if (tag != nullptr) *tag = false;
	auto* resolveStack = new std::stack<torasu::tstd::LIcore_logger_logstore::StoreGroup*>();
	if (pathDepth == 0) pathDepth = path.size();

	StoreGroup* cGroup = &tree;
	auto it = path.rbegin();
	for (size_t i = pathDepth; i > 0; i--) {
		LogId logId = *it;
		it++;
		auto& sub = cGroup->sub;
		auto found = sub.find(logId);

		if (found == sub.end()) {
			if (tag != nullptr && i == 1) { // Lowest level to be evalulated and is not found may be a tag
				*tag = true;
				break;
			}
			for (auto group : sub) {
				std::cout << " NOT " << logId << " BUT " << group.first << " @ " << std::to_string(pathDepth-i) << std::endl;
			}
			resolveStack->push(nullptr);
			break;
		}

		cGroup = found->second;
		resolveStack->push(cGroup);
	}

	return resolveStack;
}

LIcore_logger_logstore::StoreGroup* torasu::tstd::LIcore_logger_logstore::create(const std::vector<LogId>& path, std::string name) {
	StoreGroup* foundGroup;
	if (path.size() > 1) {
		std::unique_ptr<std::stack<StoreGroup*>> resolveStack(resolve(path, nullptr, path.size()-1));

		foundGroup = resolveStack->top();

		if (foundGroup == nullptr) {
			throw std::runtime_error("Failed to create new log-group, since parent can't be resolved! "
									 "(Creation of " + groupStackToStr(path) + " as " + name + " - parent #" + std::to_string(resolveStack->size()) + ")");
		}
	} else {
		foundGroup = &tree;
	}

	// delete automatically called
	return new StoreGroup(name, path[0], foundGroup);
}


void torasu::tstd::LIcore_logger_logstore::remove(StoreGroup* group) {
	group->owner->cleanupList.erase(group);
	delete group;
}

void torasu::tstd::LIcore_logger_logstore::StoreGroup::reown(StoreGroup* newOwner) {

	owner->cleanupList.erase(this);
	owner = newOwner;
	owner->cleanupList.insert(this);

}

} // namespace torasu::tstd
