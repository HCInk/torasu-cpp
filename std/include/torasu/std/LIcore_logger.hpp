#ifndef STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
#define STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_

#include <torasu/torasu.hpp>

#include <stack>

namespace torasu::tstd {

class LIcore_logger_logstore {
public:
	struct StoreGroup {
		/** @brief All sub-groups referemced under thier IDs */
		std::map<torasu::LogId, StoreGroup*> sub;
		/** @brief Which entries to be cleaned up on removal of that entry */
		std::set<StoreGroup*> cleanupList;
		/** @brief Reference to group, which owns it (has this element in thier cleanupList) */
		StoreGroup* owner = nullptr;
		/** @brief The name of the group */
		const std::string name;
		/** @brief Logs created in the group */
		std::vector<LogEntry*> logs;

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

		~StoreGroup();

	};
private:
	StoreGroup tree;

public:
	LIcore_logger_logstore();
	~LIcore_logger_logstore();

	/** @brief  Get pointers to Groups from LogId-path
	 * @param  path: The LogId path
	 * @param  pathDepth: Depth of path to read, 0 means whole path
	 * @retval The stack of the resolved groups (Stack has to be freed by caller, contents are managed)
	 */
	std::stack<StoreGroup*>* resolve(const std::vector<LogId>& path, size_t pathDepth = 0);

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
private:
	bool useAnsi = true;
	std::mutex subIdCounterLock;
	torasu::LogId subIdCounter = 0;
	LIcore_logger_logstore logstore;
	std::mutex logMutex;
public:
	LIcore_logger();
	explicit LIcore_logger(bool useAnsi);
	LogId log(LogEntry* entry, bool tag) override;
	LogId fetchSubId() override;
};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_LICORE_LOGGER_HPP_
