#ifndef STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
#define STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_

#include <map>
#include <set>
#include <vector>
#include <stack>
#include <string>

#include <torasu/torasu.hpp>

namespace torasu::tstd {

class LIcore_logger_logstore {
public:
	struct StoreGroup {
		/** @brief All sub-groups referemced under thier IDs */
		std::map<torasu::LogId, StoreGroup*> sub;
		/** @brief All tagged logs referemced under thier IDs */
		std::map<torasu::LogId, LogEntry*> tagged;
		/** @brief Which entries to be cleaned up on removal of that entry */
		std::set<StoreGroup*> cleanupList;
		/** @brief Reference to group, which owns it (has this element in thier cleanupList) */
		StoreGroup* owner = nullptr;
		/** @brief The name of the group */
		const std::string name;
		/** @brief Logs created in the group */
		std::vector<LogEntry*> logs;
		/** @brief Benchmarks of the group */
		std::vector<LogBenchmark> groupBenchmarks;
		/** @brief Tagged benchmarks */
		std::map<std::string, LogBenchmark> taggedBenchmarks;
		struct ProgressInformation {
			bool finished = false;
			bool hasInfo = false;
			int32_t total = -1;
			int32_t done = 0;
			int32_t doing = 0;
			std::string label;
			int32_t labelPos = -1;
		} progress;

		/** @brief Root constructor for tree */
		StoreGroup();

		/** @brief Constructor for nodes in the tree
		 * @note Only create manually deleted pointers of these, they will be deleted automatically
		 * @param name The name of the group
		 * @param logId The id the group is referenced under the parent
		 * @param parent The parent which the group is referenced under, and also owned by (if not reowned afterwards)
		 */
		StoreGroup(std::string name, torasu::LogId logId, StoreGroup* parent);

		/** @brief  Change owner of the group
		 * @param  newOwner: A reference to the owner
		 */
		void reown(StoreGroup* newOwner);

		/** @brief  Get pointers to Groups from LogId-path
		 * @param  path: The LogId path
		 * @param  tag: Will be set to true if object is found to be a tag, if this is set to nullptr only groups will be accepted
		 * @param  pathDepth: Depth of path to read, 0 means whole path
		 * @retval The stack of the resolved groups (Stack has to be freed by caller, contents are managed)
		 */
		std::stack<StoreGroup*>* resolve(const std::vector<LogId>& path, bool* tag = nullptr, size_t pathDepth = 0);

		~StoreGroup();

	};
private:
	StoreGroup tree;

public:
	LIcore_logger_logstore();
	~LIcore_logger_logstore();

	/** @brief  Run resolve(...) from root */
	std::stack<StoreGroup*>* resolve(const std::vector<LogId>& path, bool* tag = nullptr, size_t pathDepth = 0) {
		return tree.resolve(path, tag, pathDepth);
	}

	/**
	 * @brief  Create LogGroup at given path
	 * @param  path: The LogId path of the new group
	 * @param  name: The name of the new group
	 * @retval The group that has been created (Reference is managed)
	 */
	LIcore_logger_logstore::StoreGroup* create(const std::vector<LogId>& path, std::string name);

	/**
	 * @brief  Remove a certain log-group
	 * @param  group: The path of the log-group that should be removed
	 */
	void remove(StoreGroup* group);

	/**
	 * @retval The root-log-group, use when the log-path is empty
	 */
	inline StoreGroup* getRoot() {
		return &tree;
	}

};

class LIcore_logger : public torasu::LogInterface {
public:
	enum LogDisplayMode {
		/** @brief Updating statusbar with the progress-messages hidden (also colored) */
		FANCY = 30,
		/** @brief Updating statusbar with every message printed. (also colored) */
		FANCY_ALL = 25,
		/** @brief Printed messages colored with ANSI-colors */
		BASIC_CLOLORED = 20,
		/** @brief Basic messages - uncolored */
		BASIC = 10,
		/** @brief (TODO) Print messages as json */
		JSON = 0
	};
private:
	LogDisplayMode dispMode;
	std::mutex subIdCounterLock;
	torasu::LogId subIdCounter = 0;
	LIcore_logger_logstore logstore;
	std::mutex logMutex;
	const std::string* currentStatus = nullptr;
	size_t statusDispLength = 0;
private:
	void println(const std::string& str);
	void setStatus(const std::string* newStatus, size_t statusDispLength);
	int32_t getTerminalWidth();
public:
	explicit LIcore_logger(LogDisplayMode dispMode = FANCY);
	~LIcore_logger();
	void log(LogEntry* entry) override;
	LogId fetchSubId() override;
	std::vector<LogId>* pathFromParent(LogInterface* parent) const override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
