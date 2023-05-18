/*
 * torasu.h
 *
 *  Created on: Mar 7, 2020
 */

#ifndef CORE_INCLUDE_TORASU_TORASU_HPP_
#define CORE_INCLUDE_TORASU_TORASU_HPP_

#include <string>
#include <cstring>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stdexcept>
#include <functional>
#include <mutex>
#include <memory>
#include <optional>

// Error if a property has been provided, but hasn't been removed from the requests
#define TORASU_CHECK_FALSE_EJECTION

// Error if a render is enqueued with a null render context
#define TORASU_CHECK_RENDER_NULL_RCTX

// Error if objects which support manual initialisation via a seperate functions are initialized twice
#define TORASU_CHECK_DOUBLE_INIT

// Error if the format array has not been set
#define TORASU_CHECK_RENDER_NULL_FORMAT true

int TORASU_check_core();

namespace torasu {

// HELPER (MSIC)
typedef std::function<void(void)> Callback;

// DATA
struct Identifier;
class DataDump;
class DataResource;
class DataResourceMask;
class DataResourceHolder;

// LOGGING
typedef uint64_t LogId;
/** @brief  Maximum value of logging,
 * if the a LogId is set as this it can be counted as "NOT SET" */
inline LogId LogId_MAX = UINT64_MAX;
class LogInfoRef;
class LogEntry;
class LogInterface;
struct LogInstruction;

// HELPER (INTERFACES)
typedef uint64_t LockId;

// INTERFACES
class ExecutionInterface;


// TREE
class Element;
class Renderable;

// HELPER (TREE)
class ElementExecutionOpaque;

// DOWNSTREAM (RENDER)
class RenderInstruction;
typedef std::map<std::string, DataResource*> RenderContext; // TODO "Real" RenderContext
class ResultFormatSettings;
class ResultSettings;

// UPSTREAM (RENDER)
class RenderResult;
class RenderContextMask;

// DOWNSTREAM (READY)
class ReadyInstruction;

// UPSTREAM (READY)
class ReadyState;

// DISCOVERY
struct UserLabel;
class DiscoveryInterface;
/** @breif function, which returns a discovery-interface for a torasu-module */
typedef const DiscoveryInterface* (*DiscoveryFunction)(void);
class FactoryBase;
class DataResourceFactory;
class ElementFactory;

//
// DATA
//

struct Identifier {
	const char* str;

	/* implicit */ inline Identifier(const char* str) : str(str) {}

	inline bool operator==(const char* other) {
		return strcmp(str, other) == 0;
	}

	inline bool operator==(Identifier other) {
		return strcmp(str, other.str) == 0;
	}
};

static inline std::string operator+(std::string left, Identifier right) {
	return left + right.str;
}

static inline std::string operator+(Identifier left, std::string right) {
	return left.str + right;
}

static inline std::string operator+(const char* left, Identifier right) {
	return std::string(left) + right.str;
}

static inline std::string operator+(Identifier left, const char* right) {
	return left.str + std::string(right);
}

union DDDataPointer {
	unsigned char* b;
	const char* s;
};

class DataDump {
private:
	DDDataPointer data;
	int size;
	std::function<void(DataDump*)>* freeFunc;
	bool json;
public:
	inline DataDump(DDDataPointer data, int size, std::function<void(DataDump*)>* freeFunc, bool isJson = false)
		: data(data), size(size), freeFunc(freeFunc), json(isJson) {}

	~DataDump() {
		if (freeFunc != nullptr) {
			(*freeFunc)(this);
		}
	}

	inline DDDataPointer const getData() const {
		return data;
	}

	inline int const getSize() const {
		return size;
	}

	inline bool isJson() const {
		return json;
	}

	inline DataDump* newReference() const {
		return new DataDump(data, size, nullptr, json);
	}
};

class DataResource {
public:
	enum CompareResult {
		/** @brief The first object is less then the second object */
		LESS = -1,
		/** @brief The first object is equal to the second object */
		EQUAL = 0,
		/** @brief The first object is greater then the second object */
		GREATER = 1,
		/** @brief The objects can't be compared since the types are different */
		TYPE_ERR = 3
	};

	DataResource() {}
	virtual ~DataResource() {}

	virtual Identifier getType() const = 0;
	virtual DataDump* dumpResource() = 0;
	virtual DataResource* clone() const = 0;

	virtual CompareResult compare(const DataResource* other) const {
		if (this == other) return EQUAL;

		if (this->getType() == other->getType()) {
			return (other > this) ? GREATER : LESS;
		} else {
			return TYPE_ERR;
		}
	}
};

class DataResourceMask : public DataResource {
public:
	enum MaskCompareResult {
		/** @brief The mask approves that the checked object is contained */
		MCR_INSIDE,
		/** @brief The mask denies that the checked object is contained */
		MCR_OUTSIDE,
		/** @brief The mask can't check if the object is contained or not */
		MCR_UNKNOWN
	};
	DataResourceMask() {}
	virtual ~DataResourceMask() {}

	/** @brief  Check containment of other object
	 * @param  obj: The object that shall be checked against it
	 * @retval The MaskCompareResult of the comparison of both */
	virtual MaskCompareResult check(const DataResource* obj) const = 0;

	/** @brief  Merge two masks, into one which contains the intersections
	 * @param  other: The other mask to be merged with
	 * @retval The newly generated mask, nullptr if not mergable (Freed/Managed by caller) */
	virtual DataResourceMask* merge(const DataResourceMask* other) const = 0;


	virtual DataResourceMask* clone() const = 0;

	virtual bool isUnknown() const {
		return false;
	}

	class DataResourceMaskUnknown;
	class DataResourceMaskSingle;
};

class DataResourceMask::DataResourceMaskUnknown : public DataResourceMask {
public:
	DataResourceMaskUnknown() {}
	~DataResourceMaskUnknown() {}

	Identifier getType() const override {
		return "T::DRMU";
	}

	DataDump* dumpResource() override {
		return nullptr;
	}

	DataResourceMaskUnknown* clone() const override {
		return new DataResourceMaskUnknown();
	}

	MaskCompareResult check(const DataResource* obj) const override {
		return MaskCompareResult::MCR_UNKNOWN;
	}

	DataResourceMask* merge(const DataResourceMask* other) const override {
		return clone();
	}

	bool isUnknown() const override {
		return false;
	}
};

class DataResourceMask::DataResourceMaskSingle : public DataResourceMask {
private:
	DataResource* dr;
public:
	explicit DataResourceMaskSingle(DataResource* dr) : dr(dr) {}
	~DataResourceMaskSingle() {
		delete dr;
	}

	Identifier getType() const override {
		return "T::DRMS";
	}

	DataDump* dumpResource() override {
		return nullptr; // TODO Nested DPs
	}

	DataResourceMaskSingle* clone() const override {
		return new DataResourceMaskSingle(dr != nullptr ? dr->clone() : nullptr);
	}

	MaskCompareResult check(const DataResource* obj) const override {
		return dr != nullptr ? (dr->compare(obj) == EQUAL ? MCR_INSIDE : MCR_OUTSIDE)
			   : (obj == nullptr ? MCR_INSIDE : MCR_OUTSIDE);
	}

	DataResourceMask* merge(const DataResourceMask* other) const override {
		if (other->check(dr) == MCR_INSIDE) {
			return clone();
		} else {
			return new DataResourceMaskUnknown();
		}
	}
};

class DataResourceHolder {
private:
	DataResource* dr;
	bool owning;

public:
	DataResourceHolder() : dr(nullptr), owning(false) {}
	DataResourceHolder(DataResource* dr, bool owning) : dr(dr), owning(owning) {}
	~DataResourceHolder() {
		if (owns()) {
			delete dr;
		}
	}

	inline void initialize(DataResource* dr, bool owning) {
#ifdef TORASU_CHECK_DOUBLE_INIT
		if (this->dr != nullptr) throw std::logic_error("DataResourceHolder may not be intialized twice!");
#endif
		this->dr = dr;
		this->owning = owning;
	}

	inline bool owns() const {
		return owning;
	}

	inline DataResource* const get() const {
		return dr;
	}

	/**
	 * @brief  Ejects the data to take control of manual memory management
	 * @note   Only available if owns() = true
	 * @retval The pointer to the result that has been ejected
	 */
	inline DataResource* const eject() {
#ifdef TORASU_CHECK_FALSE_EJECTION
		if (!owns()) {
			throw std::runtime_error("Ejected result that wasn't ejectable - owns() = false");
		}
#endif
		owning = false;
		return dr;
	}



};

//
// LOGGING
//

// Defines that logging modules are available
#define TORASU_REG_LOGGING

class LogInfoRef {
public:
	/** @brief  Group containing information about generation of the result */
	std::vector<LogId>* const groupRef;
	/** @brief  References to causes (relative to group) */
	const std::vector<std::vector<LogId>>* const causeRefs;


	explicit LogInfoRef(const LogInfoRef& src)
		: groupRef(new auto(*src.groupRef)), causeRefs(new auto(*src.causeRefs)) {}

	explicit LogInfoRef(std::vector<LogId>* groupRef)
		: groupRef(groupRef), causeRefs(new std::vector<std::vector<LogId>>()) {}

	explicit LogInfoRef(const std::vector<std::vector<LogId>>* causeRefs)
		: groupRef(new std::vector<LogId>()), causeRefs(causeRefs) {}

	LogInfoRef(std::vector<LogId>* groupRef, const std::vector<std::vector<LogId>>* causeRefs)
		: groupRef(groupRef), causeRefs(causeRefs) {}

	~LogInfoRef() {
		delete groupRef;
		delete causeRefs;
	}
};

enum LogType {
	/** @brief This log-entry is a normal message
	 *  @note Indicates that object is of type torasu::LogMessage - usage without being of that type will lead to undefined behavior */
	LT_MESSAGE = 0,
	/** @brief This log-entry indicates a new log-group (beginning now log entries can be expected from the group)
	 * @note Indicates that object is of type torasu::LogGroupStart - usage without being of that type will lead to undefined behavior */
	LT_GROUP_START = 10,
	/** @brief This log-entry indicates the end of a log-group-report (no more new entries from that group will be spawned from now on) */
	LT_GROUP_END = 11,
	/** @brief Will notify that all log messages below will nologer be referenced
	* - except log messages inside groups, which have been called LT_GROUP_PERSIST
	* - those have to explicity called with this (LT_GROUP_UNREF) to be dereferenced */
	LT_GROUP_UNREF = 12,
	/** @brief Will mark that group-contents shall be persisted, even if an above group gets LT_GROUP_UNREF.
	 * To dereference LT_GROUP_UNREF has to be called explicity on the group */
	LT_GROUP_PERSIST = 13,
	/** @brief Contains data that shall be logged
	 * @note Indicates that object is of type torasu::LogData - usage without being of that type will lead to undefined behavior */
	LT_DATA = 20,
	/** @brief Contains information about how long things have taken / how many resources have been used for certain application
	 * @note Indicates that object is of type torasu::LogBenchmark - usage without being of that type will lead to undefined behavior */
	LT_BENCHMARK = 21,
	/** @brief Contains information about the progress of the group
	 * @note Indicates that object is of type torasu::LogProgress - usage without being of that type will lead to undefined behavior */
	LT_PROGRESS = 22,
	/** @brief Failed to determine the type of the log-entry */
	LT_UNKNOWN = -1
};

enum LogLevel {
	/** @brief Used for small details, lowest log-level */
	TRACE = -50,
	/** @brief Used for frequent debug-information */
	DEBUG = -20,
	/** @brief Used for information about configuration and other unfrequent information */
	INFO = -10,
	/** @brief Used to warn about misuse or possible errors, default log-level */
	WARN = 0,
	/** @brief Used to log definite errors */
	ERROR = 10,
	/** @brief Used to indicate an error, which interrupts the rendering-process */
	SERVERE_ERROR = 20,
	/** @brief Failed to determine the log-level of the message */
	LEVEL_UNKNOWN = 99,
	/** @breif No log messages at all, logging at this level (or higher) can lead to undefined behavior */
	NO_LOGGING = 100
};

/**
 * @brief  Packed version of the LogEntry
 * @note   Spoiled in torasu/torasu.hpp, implemented in torasu/Dlog_entry.hpp
 */
class Dlog_entry;

/**
 * @brief  Entry to be logged
 */
class LogEntry {
public:
	const LogType type;
	/** @brief  Grouping-stack, from source to root
	 * @note Used for log-grouping, don't touch if you dont know what you are doing
	 * - usually only touched by logging-interfaces */
	std::vector<LogId> groupStack;

	explicit LogEntry(LogType type)
		: type(type) {}

	virtual Dlog_entry* makePack();

	inline LogId addTag(torasu::LogInterface* li);

	virtual ~LogEntry() {}
};

/**
 * @brief  Message to be logged
 */
class LogGroupStart : public LogEntry {
public:
	const std::string name;

	explicit LogGroupStart(std::string name)
		: LogEntry(LogType::LT_GROUP_START), name(name) {}
};

/**
 * @brief  Message to be logged
 */
class LogMessage : public LogEntry {
public:
	const LogLevel level;
	const std::string text;
	const LogInfoRef* info;

	LogMessage(LogLevel level, std::string message, const LogInfoRef* info=nullptr)
		: LogEntry(LogType::LT_MESSAGE), level(level), text(message), info(info) {}

	~LogMessage() {
		if (info != nullptr) delete info;
	}
};

/**
 * @brief  Data to be logged
 */
class LogData : public LogEntry {
public:
	const std::string text;
	const DataResource* data;

	LogData(std::string text, const DataResource* data)
		: LogEntry(LogType::LT_DATA), text(text), data(data) {}

	~LogData() {
		if (data != nullptr) delete data;
	}
};

/**
 * @brief  Benchmark to be logged
 */
class LogBenchmark : public LogEntry {
public:
	/** @brief  Unit for storing benchmark-times */
	typedef uint64_t bench_t;
	/** @brief  Maximum value of bench_t / Also indicates that a value may not have been set */
	inline static const bench_t bench_t_MAX = UINT64_MAX;

	/** @brief  Selects which type of benchmak this is
	 * and how BenchInfo should be interpreted */
	enum BenchType {
		/** @brief Tagged benchmark, describes a single operation with a tag
		 * - OpInfo shall be interpreted as opTag, which is the tag/label of the operation */
		BenchType_TAGGED = 0,
		/** @brief Benchmark of the group the log message is in
		 * - OpInfo shall be interpreted as toContinue, which is indicates if a followup-benchmark
		 * continues the benchmark (true) or if this has been the last benchmark for the group (false) */
		BenchType_GROUP = 1
	} const benchType;

	union BenchInfo {
		/** @brief  Tag of operation (When using BenchType_TAGGED) */
		std::string* opTag;
		inline BenchInfo(std::string* opTag) : opTag(opTag) {}
		/** @brief  Weather the operation is just supsneded and will be continued (true),
		 * or if this is the last benchmark-section regarding this group (false) (When using BenchType_GROUP) */
		bool toContinue;
		inline BenchInfo(bool toContinue) : toContinue(toContinue) {}
	} const benchInfo;

	/** @brief  CPU-time used in 10^-6sec */
	const bench_t calcTime;
	/** @brief  Actual time the operation took in 10^-6sec */
	const bench_t elapsed;
	/** @brief  Time in 10^-6sec */
	const bench_t position;

private:
	/** @brief Contstructor for group-benchmark, see createGroupBenchmark(...) for more info */
	LogBenchmark(bench_t calcTime, bench_t elapsed, bench_t position, bool toContinue)
		: LogEntry(LogType::LT_BENCHMARK), benchType(BenchType_GROUP),
		  benchInfo(toContinue), calcTime(calcTime), elapsed(elapsed), position(position) {}

public:
	/**
	 * @brief  Create a log-message, which notes the performance of a certain operation (identified by the opTag)
	 * @param  opTag: The tag the operation should be tagged with
	 * @param  calcTime: The CPU-time the operation took (in 10^-6sec)
	 * @param  elapsed: The actual time the operation took
	 * @param  position: The position in time the operation began (in 10^-6sec) - if unknwon set to bench_t_MAX
	 */
	LogBenchmark(const std::string& opTag, bench_t calcTime, bench_t elapsed, bench_t position)
		: LogEntry(LogType::LT_BENCHMARK), benchType(BenchType_TAGGED),
		  benchInfo(new std::string(opTag)), calcTime(calcTime), elapsed(elapsed), position(position) {}

	/**
	 * @brief  Create a log-message, which notes the performance of the group it is sent to
	 * @note   This type of log-entry is intended log the performance of the operations done in the group,
	 * 			therefore this may only be written to groups you have created yourself
	 * 			or groups you recieved explicit permission to write this to.
	 * 			Be careful when writing to groups/loginterfaces you have not created to not create any colisions.
	 * @param  calcTime: The CPU-time the operation took (in 10^-6sec)
	 * @param  elapsed: The actual time the operation took
	 * @param  position: The position in time the operation began (in 10^-6sec) - if unknwon set to bench_t_MAX
	 * @param  toContinue: true: there will be followup messages contuing the report,
	 * 					  	false: this is the last benchmark regarding this group
	 * @retval The created bench-LogEntry, needs to be freed by caller
	 */
	inline static LogBenchmark* createGroupBenchmark(bench_t calcTime, bench_t elapsed, bench_t position = bench_t_MAX, bool toContinue = false) {
		return new LogBenchmark(calcTime, elapsed, position, toContinue);
	}

	~LogBenchmark() {
		if (benchType == BenchType_TAGGED && benchInfo.opTag != nullptr) delete benchInfo.opTag;
	}
};


/**
 * @brief  Progress to be logged
 */
class LogProgress : public LogEntry {
public:
	/** @brief  The total number of units (-1 if unknown) */
	const int64_t total;
	/** @brief  How many tasks are done being processed (Range: 0 <= done <= total) */
	const int64_t done;
	/** @brief  How many tasks are currently being processed (Range: 0 <= done+pending <= total) */
	const int64_t doing;
	/** @brief  Label of the progress-entry at the position of done+pending (Only valid if pending > 0) */
	const std::string label;

	/**
	 * @brief  Creates a log entry which indicates how much progress has been made
	 * @param  total: The total number of units (-1 if unknown)
	 * @param  done: How many tasks are done being processed (Range: 0 <= done <= total)
	 * @param  doing: How many tasks are currently being processed (Range: 0 <= done+pending <= total)
	 * @param  label: Label of the progress-entry at the position of done+pending (only valid if pending > 0)
	 */
	LogProgress(int64_t total, int64_t done, int64_t doing, std::string label = "")
		: LogEntry(LogType::LT_PROGRESS), total(total), done(done), doing(doing), label(label) {}

	/**
	 * @brief  Creates a log entry which indicates how much progress has been made (with pending = 1)
	 * @param  total: The total number of units (-1 if unknown)
	 * @param  done: How many tasks are done being processed (Range: 0 <= done <= total)
	 * @param  label: Label of the progress-entry at the position of done+1 (only valid if done < total)
	 */
	LogProgress(int64_t total, int64_t done, std::string label = "")
		: LogEntry(LogType::LT_PROGRESS), total(total), done(done), doing(done < total ? 1 : 0), label(label) {}

};

class LogInterface {
public:
	/**
	 * @brief  Logs an entry to the logging system
	 * @param  level The log level of the entry
	 * @param  msg The message of the entry
	 */
	inline void log(LogLevel level, std::string msg) {
		log(new LogMessage(level, msg));
	}

	/**
	 * @brief  Logs an entry to the logging system
	 * @param  entry: The entry to be logged (will be managed by the interface)
	 */
	virtual void log(LogEntry* entry) = 0;

	/**
	 * @brief  Generates a new Subgroup-ID
	 * @retval Id of Subgroup
	 */
	virtual LogId fetchSubId() = 0;

	/**
	 * @brief  Generates path from parent-log-interface to current
	 * @return relative path of log-ids from given parent to this interface
	 * 	(NOTE: Please be aware this in reverse compared to the usual notation)
	 * 	- Will return nullptr, when parent could not be found
	 * 		  or the interface has no id, so no path even exists (often when there were no messages sent)
	 */
	virtual std::vector<LogId>* pathFromParent(LogInterface* parent) const = 0;

	virtual ~LogInterface() {}
};

inline LogId LogEntry::addTag(torasu::LogInterface* li) {
	if (!groupStack.empty())
		throw std::logic_error("Can't add tag to LogEntry, which has already a group assigned.");
	LogId tag = li->fetchSubId();
	groupStack.push_back(tag);
	return tag;
}

/**
 * @brief  Tells the process, which gets this how messages should be logged
 */
struct LogInstruction {
	/** @brief  Interface to send the log-messages to */
	LogInterface* logger;

	/** @brief The minimum level of log messages that should be recorded  */
	LogLevel level;

	/** @brief Enabled options */
	uint64_t options;

	/** @brief  Tells the runner to log total benchmarks of executions */
	inline static uint64_t OPT_RUNNER_BENCH = 0x1;
	/** @brief  Tells the runner to log detailed benchmarks of executions
	 * @note Needs to be combined with OPT_RUNNER_BENCH to be effective */
	inline static uint64_t OPT_RUNNER_BENCH_DETAILED = 0x2;
	/** @brief  When set progress shall be reported */
	inline static uint64_t OPT_PROGRESS = 0x4;

	explicit LogInstruction(LogInterface* logger, LogLevel level = LogLevel::WARN, uint64_t options = 0x0) : logger(logger), level(level), options(options) {}
};

//
// INTERFACES
//

/**
 * @brief  Interface to run tasks over, like rendering a Renderable
 */
class ExecutionInterface {
public:

	/** Struct for fetching results via fetchRenderResults(ResultPair*,size_t) */
	struct ResultPair {
		/** renderId which should be fetched, this should be set when making the request */
		uint64_t renderId;
		/** The RenderResult of the render, this is filled by the function */
		RenderResult* result = nullptr;
	};

	virtual ~ExecutionInterface() {}

	/**
	 * @brief  Enqueue a renderable to be processed asynchronously
	 * @note   This oepration is usually expected to not take long, since it usually just enqueues a task
	 * @param  rend: The renderable to be enqueued
	 * @param  rctx: The context the renderable should be executed with
	 * @param  rs: The result-resttings of the render-operation
	 * @param  logger: Log instruction the operation should be logged with
	 * @param  prio: the local priority of execution
	 * @retval The renderId to be used to retrieve the result via fetchRenderResult(uint64_t)
	 */
	virtual uint64_t enqueueRender(Renderable* rend, RenderContext* rctx, ResultSettings* rs, LogInstruction li, int64_t prio) = 0;

	/**
	 * @brief  Retrieve render-results of previously enqueued renders, sleeps/finishes the tasks if not completed yet
	 * @note   This is only guranteed to work once for each render - this may also take more time, since it may need to finish the rendering
	 * @param  requests: List of requests, which contain the renderId of the enqueued render, this will fill the result-field accordingly
	 * @param  requestCount: Count of requests
	 */
	virtual void fetchRenderResults(ResultPair* requests, size_t requestCount) = 0;

	/**
	 * @brief  Fetch render-result if available, returns nullptr if not available
	 * @param  renderId: Render-id created in enqueueRender
	 * @retval The generated render-result, will need to be managed/freed by caller
	 */
	virtual torasu::RenderResult* tryFetchRenderResult(uint64_t renderId) = 0;

	/**
	 * @brief  Lock a section of a running render-process ( unlock with unlock(LockId) )
	 * @param  lockId: The id of the lock, in case there are multiple
	 */
	virtual void lock(LockId lockId=0) = 0;

	/**
	 * @brief  Unlock a section of a running render-process ( has to be previously locked via lock(LockId) )
	 * @param  lockId: The id of the lock, in case there are multiple
	 */
	virtual void unlock(LockId lockId=0) = 0;

	/**
	 * @brief  Retrieve render-result of a previously enqueued render, sleeps/finishes the task if not completed yet
	 * @note   This is only guranteed to work once for each render - When fetching multiple use fetchRenderResults(ResultPair*,size_t)
	 * @param  renderId: The renderId generated when running enqueueRender(Renderable*,RenderContext*,ResultSettings*,prio)
	 * @retval The RenderResult generated by the render-process, this has to be freed by the caller
	 */
	inline RenderResult* fetchRenderResult(uint64_t renderId) {
		ResultPair rp;
		rp.renderId = renderId;
		fetchRenderResults(&rp, 1);
		return rp.result;
	}

};

//
// TREE
//

class ElementExecutionOpaque {
public:
	ElementExecutionOpaque() {}
	virtual ~ElementExecutionOpaque() {}
};

template<class T> class Slot {
protected:
	T* elem;
	bool owned = false;

public:
	inline Slot()
		: elem(nullptr) {}

	/* implicit */ inline Slot(T* elem)
		: elem(elem) {}

	/** @brief  Convert slot to ElementSlot */
	inline operator Slot<torasu::Element>() {
		return Slot<torasu::Element>(elem, owned);
	}

	inline Slot(T* elem, bool owned)
		: elem(elem), owned(owned) {}

	inline T& operator*() {
		return *elem;
	}
	inline operator bool() {
		return elem != nullptr;
	}

	inline T* get() const {
		return elem;
	}

	inline bool isOwned() const {
		return owned;
	}

	virtual ~Slot() {}

};

typedef Slot<torasu::Element> ElementSlot;
typedef Slot<torasu::Renderable> RenderableSlot;

typedef std::map<std::string, ElementSlot> ElementMap;
typedef std::optional<ElementSlot> OptElementSlot;

class Element {
public:
	ElementExecutionOpaque* elementExecutionOpaque = nullptr;
	std::mutex elementExecutionOpaqueLock;

	Element() {}
	virtual ~Element() {
		if (elementExecutionOpaque != nullptr) delete elementExecutionOpaque;
	}

	virtual void ready(ReadyInstruction* ri) = 0;

	virtual Identifier getType() = 0;
	virtual DataResource* getData() = 0;
	virtual ElementMap getElements() = 0;

	virtual void setData(DataResource* data,
						 ElementMap elements) = 0;
	virtual void setData(DataResource* data) = 0;
	/**
	 * @brief  Set element-slot by key
	 * @param  key: Key of slot
	 * @param  elem: Element to be set (nullptr to reset to default or remove if no default exists)
	 * @retval Slot in the new state, nullopt if does not exist (or slot could not be created)
	 * @note When slots with owned elments are provided, ownership of the element is only transfered,
	 * 			if  this function returns an element-slot with the provided elment
	 */
	virtual const OptElementSlot setElement(std::string key, const ElementSlot* elem) = 0;

};

class Renderable : public virtual Element {
public:
	Renderable() {}
	virtual ~Renderable() {}

	virtual RenderResult* render(RenderInstruction* ri) = 0;
};

//
// DOWNSTREAM (RENDER)
//

class RenderInstruction {
private:
	RenderContext* rctx;
	ResultSettings* rs;
	ExecutionInterface* ei;
	LogInstruction li;
	ReadyState* rdys;

public:
	inline RenderInstruction(RenderContext* rctx, ResultSettings* rs, ExecutionInterface* ei, LogInstruction li, ReadyState* rdys)
		: rctx(rctx), rs(rs), ei(ei), li(li), rdys(rdys) {}

	~RenderInstruction() {}

	inline RenderContext* const getRenderContext() {
		return rctx;
	}

	inline ResultSettings* const getResultSettings() {
		return rs;
	}

	inline ExecutionInterface* const getExecutionInterface() {
		return ei;
	}

	inline LogInstruction getLogInstruction() {
		return li;
	}

	inline ReadyState* const getReadyState() {
		return rdys;
	}
};

class ResultSettings {
private:
	Identifier pipeline;
	ResultFormatSettings* const* formats;
public:
	/**
	 * @brief  Create ResultSettings for a render-task
	 * @param  pipeline: Identifier of pipeline
	 * @param  formats[]: Format vector (null-terminated), the earlier the higher the priority
	 * 					  - note that the priority may still be ignored by a renderable,
	 * 						 for example when another suggested format is more fitting
	 */
	inline ResultSettings(Identifier pipeline, ResultFormatSettings* const formats[])
		: pipeline(pipeline), formats(formats) {
#if TORASU_CHECK_RENDER_NULL_FORMAT
		if (formats == nullptr) throw std::logic_error("ResultSettings: format-list may not be null!");
#endif
	}

	~ResultSettings() {}

	inline Identifier getPipeline() const {
		return pipeline;
	}

	inline ResultFormatSettings* const* getFormats() const {
		return formats;
	}
};

#define TORASU_FORMAT_PREFIX "F#"

class ResultFormatSettings : public virtual DataResource {
private:
	std::string ident;
public:
	explicit ResultFormatSettings(std::string dataType)
		: ident(TORASU_FORMAT_PREFIX + dataType) {}
	virtual ~ResultFormatSettings() {}

	Identifier getType() const override {
		return ident.c_str();
	}

	virtual std::set<std::string> const getTags() {
		return std::set<std::string>();
	}

};

//
// Upstream (RENDER)
//

enum RenderResultStatus {
	/** @brief The given format is not available */
	RenderResultStatus_INVALID_FORMAT = -5,

	/** @brief The given segment was not provided
	 * @note This is reserved for wrappers, to signalize,
	 * 	that the Segment is completely absent from the given ResultSettings.
	 * 	This should never be returned in a first-party generated RenderResult,
	 * 	if you want to signaize, that a segment is not supported,
	 * 	then use RenderResultStatus_INVALID_SEGMENT serves that purpose */
	RenderResultStatus_NON_EXISTENT = -4,

	/** @brief The given segment is invalid */
	RenderResultStatus_INVALID_SEGMENT = -3,

	/** @brief An internal error or undefined behavior
	 * occurred during the execution */
	RenderResultStatus_INTERNAL_ERROR = -1,

	/** @brief Everything went as expected */
	RenderResultStatus_OK = 0,

	/** @brief Warnings were thrown while running */
	RenderResultStatus_OK_WARN = 2
};

class RenderContextMask {
public:
	std::map<std::string, DataResourceMask*>* maskMap;
	bool freeMapContents = true;

	RenderContextMask() : maskMap(new std::map<std::string, DataResourceMask*>()) {}
	RenderContextMask(const RenderContextMask& orig) : maskMap(new auto(*orig.maskMap)) {
		for (auto& entry : *maskMap)
			entry.second = entry.second != nullptr ? entry.second->clone() : nullptr;
	}

	explicit RenderContextMask(std::map<std::string, DataResourceMask*>* maskMap, bool freeMapContents = true)
		: maskMap(maskMap), freeMapContents(freeMapContents) {}

	~RenderContextMask() {
		if (freeMapContents) {
			for (auto& entry : *maskMap) {
				if (entry.second != nullptr) delete entry.second;
			}
		}
		delete maskMap;
	}

	/** @brief  Filters mask - Removes entries with given keys
	 * @param  toFilter: The keys of the entries to be removed
	 * @retval The filtered mask using references - thereforce the original RenderContextMask
	 * 			should be kept valid during the existence of the filtered mask */
	inline RenderContextMask* filter(std::vector<std::string> toFilter) const {
		auto* refClone = new RenderContextMask(new std::map<std::string, DataResourceMask*>(*maskMap), false);

		for (const auto& currKey : toFilter)
			refClone->maskMap->erase(currKey);

		return refClone;
	}

	inline DataResourceMask::MaskCompareResult check(RenderContext* rctx) const {
		DataResourceMask::MaskCompareResult res = DataResourceMask::MCR_INSIDE;
		auto mapEnd = maskMap->end();
		for (auto rctxEntry : *rctx) {
			auto found = maskMap->find(rctxEntry.first);
			if (found != mapEnd) {
				switch (found->second->check(rctxEntry.second)) {
				case DataResourceMask::MCR_INSIDE:
					break;
				case DataResourceMask::MCR_OUTSIDE: {
						if (res == DataResourceMask::MCR_INSIDE)
							res = DataResourceMask::MCR_OUTSIDE;
						break;
					}
				case DataResourceMask::MCR_UNKNOWN:
				default: {
						res = DataResourceMask::MCR_UNKNOWN;
						break;
					}
				}
			}
		}
		return res;
	}

	/** @brief  Limits the mask of the current object to the other object */
	inline void mergeInto(const RenderContextMask& other) {
		auto thisEnd = maskMap->end();
		auto otherEnd = other.maskMap->end();
		for (auto& entry : *maskMap) {
			auto found = other.maskMap->find(entry.first);
			if (found != otherEnd) { // In both a and b
				auto* oldMask = entry.second;
				auto* newMask = oldMask->merge(found->second);
				if (newMask == nullptr) {
					newMask = new DataResourceMask::DataResourceMaskUnknown();
				}
				entry.second = newMask;
				delete oldMask;
			}
		}

		for (const auto& entry : *other.maskMap) {
			auto found = maskMap->find(entry.first);
			if (found == thisEnd) { // In only b
				(*maskMap)[entry.first] = entry.second->clone();
			}
		}
	}

	/** @brief  Creates intersection of two RenderContextMask-objects
	 * @retval Mask which containes the common areas between the two input masks (managed by caller) */
	static inline RenderContextMask* merge(const RenderContextMask& a, const RenderContextMask& b) {
		auto* mask = new RenderContextMask();
		auto& maskMap = *mask->maskMap;
		auto aEnd = a.maskMap->end();
		auto bEnd = b.maskMap->end();
		for (const auto& entry : *a.maskMap) {
			DataResourceMask* valueMask;
			auto found = b.maskMap->find(entry.first);
			if (found != bEnd) { // In both a and b
				valueMask = entry.second->merge(found->second);
				if (valueMask == nullptr) {
					valueMask = new DataResourceMask::DataResourceMaskUnknown();
				}
			} else { // Only in a
				valueMask = entry.second->clone();
			}
			maskMap[entry.first] = valueMask;
		}

		for (const auto& entry : *b.maskMap) {
			auto found = a.maskMap->find(entry.first);
			if (found == aEnd) { // In only b
				maskMap[entry.first] = entry.second->clone();
			}
		}

		return mask;
	}

	/** @brief  Creates intersection of n RenderContextMask-objects
	 * @param  masks: List of pointers to masks to be merged
	 * @retval Mask which containes the common areas between the two input masks (managed by caller) */
	static inline RenderContextMask* merge(std::initializer_list<const RenderContextMask*> masks) {

		switch (masks.size()) {
		case 0:
			return new RenderContextMask();
		case 1:
			return new RenderContextMask(**masks.begin());
		default:
			break;
		}

		auto it = masks.begin();

		RenderContextMask* mask;
		{
			auto* a = *it;
			it++;
			mask = merge(*a, **it);
			it++;
		}

		for (auto end = masks.end(); it != end; it++)
			mask->mergeInto(**it);

		return mask;
	}


	inline void resolveUnknownsFromRctx(torasu::RenderContext* rctx) {
		for (auto maskEntry : *maskMap) {
			if (maskEntry.second->isUnknown()) {
				delete maskEntry.second;
				maskEntry.second = new DataResourceMask::DataResourceMaskSingle((*rctx)[maskEntry.first]->clone());
			}
		}
	}

};

class RenderResult {
private:
	RenderResultStatus status;
	DataResourceHolder result;
	RenderContextMask* rctxm = nullptr;
	LogInfoRef* rir;

	/** @brief  Will be called, when destructed,
	 * 	@note	May be overriden by a cleanup-callback which includes the cleanup
	 * 			of a group containing the previous group */
	Callback* refCloseFunc = nullptr;

	/** @brief  The LogInterface the groups referenced are relative to
	 * @note 	This points to the LogInterface the render-task has been run with.
	 * 			It may nolonger be be valid depending on how the LogInterface is handled */
	LogInterface* li = nullptr;

public:
	/**
	 * @brief  Creates a RenderResult (only status, without content)
	 * @note  This constructor should only be used if a result wasn't generated
	 * @param  status: Calculation-status of the segment
	 * @param  rir: References to Information in the Logs about the result (shall be valid until the callback containing this is called with RIR_UNREF)
	 */
	explicit inline RenderResult(RenderResultStatus status, LogInfoRef* rir = nullptr)
		: status(status), result(), rir(rir) {}

	/**
	 * @brief  Creates a RenderResult
	 * @param  status: Calculation-status of the segment
	 * @param  result: The result of the calculation of the segment
	 * @param  freeResult: Flag to destruct the DataResource of the result (true=will destruct)
	 * @param  rir: References to Information in the Logs about the result (shall be valid until the callback containing this is called with RIR_UNREF)
	 */
	inline RenderResult(RenderResultStatus status, DataResource* result, bool freeResult, LogInfoRef* rir = nullptr)
		: status(status), result(result, freeResult), rir(rir) {}

	/**
	 * @brief  Creates a RenderResult (only status, without content)
	 * @note  This constructor should only be used if a result wasn't generated
	 * @param  status: Calculation-status of the segment
	 * @param  rctxm: The RenderContext-range this status is valid in
	 * @param  rir: References to Information in the Logs about the result (shall be valid until the callback containing this is called with RIR_UNREF)
	 */
	inline RenderResult(RenderResultStatus status, RenderContextMask* rctxm, LogInfoRef* rir = nullptr)
		: status(status), rctxm(rctxm), rir(rir) {}

	/**
	 * @brief  Creates a RenderResult
	 * @param  status: Calculation-status of the segment
	 * @param  result: The result of the calculation of the segment
	 * @param  freeResult: Flag to destruct the DataResource of the result (true=will destruct)
	 * @param  rctxm: The RenderContext-range this result is valid in
	 * @param  rir: References to Information in the Logs about the result (shall be valid until the callback containing this is called with RIR_UNREF)
	 */
	inline RenderResult(RenderResultStatus status, DataResource* result, bool freeResult, RenderContextMask* rctxm, LogInfoRef* rir = nullptr)
		: status(status), result(result, freeResult), rctxm(rctxm), rir(rir) {}

	~RenderResult() {
		if (rctxm != nullptr) delete rctxm;
		if (rir != nullptr) delete rir;
		closeRefs();
	}

	inline RenderResultStatus const getStatus() {
		return status;
	}

	inline DataResource* const getResult() {
		return result.get();
	}

	inline const RenderContextMask* getResultMask() const {
		return rctxm;
	}

	inline void updateResultMask(RenderContextMask* rctxm) {
		this->rctxm = rctxm;
	}

	inline LogInfoRef* getResultInfoRef() {
		return rir;
	}

	inline bool const canFreeResult() {
		return result.owns();
	}

	/**
	 * @brief  Ejects result to take control of manual memory management
	 * @note   Only available if canFreeResult() = true
	 * @retval The pointer to the result that has been ejected
	 */
	inline DataResource* const ejectResult() {
		return result.eject();
	}

	inline DataResource* const ejectOrClone() {
		return result.owns() ? result.eject() : result.get()->clone();
	}

	inline LogInterface* const getLogInterface() {
		return li;
	}

	/** @brief  Closes all log-refs in contained segments, the LogInterface can then be freed after calling this
	 * @note This will also be called on ~RenderResult, so only call this when the LogInterace the render-operation is executed with
	 * 		shall be cleaned up earlier then the RenderResult */
	inline void closeRefs() {
		if (refCloseFunc == nullptr) return;
		(*refCloseFunc)();
		delete refCloseFunc;
		refCloseFunc = nullptr;
	}

	/**
	 * @brief  Updates relative references in log-interface
	 * @param  liNew: The new LogInterface to be set
	 * @param  refCloseFuncNew: The new close-callback
	 */
	inline void reRefResult(LogInterface* liNew, Callback* refCloseFuncNew = nullptr) {
		if (li != nullptr) {
			std::unique_ptr<std::vector<LogId>> path(li->pathFromParent(liNew));
			reRefResult(liNew, const_cast<const std::vector<LogId>*>(path.get()), refCloseFuncNew);

			// Sanity-checking
			if (path == nullptr) {
				if (getResultInfoRef() != nullptr)
					throw std::logic_error("RenderResult contains info-refs even though the path is not established!"
										   " (Hint: ResultInfoRefs may only be set if there have been messages sent"
										   " - Hint: May also be caused by an invalid new LogInterface, which is not parent of the current)");
			}

		} else {
			reRefResult(liNew, nullptr, refCloseFuncNew);
		}

	}

	/**
	 * @brief  Updates relative references in log-interface
	 * @param  liNew: The new LogInterface to be set
	 * @param  path: Path from new to old interface
	 * @param  refCloseFuncNew: The new close-callback
	 */
	inline void reRefResult(LogInterface* liNew, const std::vector<LogId>* path, Callback* refCloseFuncNew = nullptr) {
		if (path != nullptr) {
			if (rir == nullptr) rir = new LogInfoRef(new std::vector<LogId>());
			const LogId* pathIt = path->data() + path->size() - 1;
			for (size_t i = path->size(); i > 0; i--) {
				rir->groupRef->push_back(*pathIt);
				pathIt--;
			}
		}

		li = liNew;
		if (refCloseFunc != nullptr) delete refCloseFunc;
		refCloseFunc = refCloseFuncNew;
	}
};

//
// DOWNSTREAM (READY)
//

class ReadyInstruction {
public:
	/** @brief  The operations/segemnts to be made ready */
	const std::vector<Identifier> ops;
	/** @brief  The target-RenderContext to be made ready towards */
	/* TODO make const */ RenderContext* const rctx;
	/** @brief  The ExecutionInterface to be used run executions required to made ready */
	ExecutionInterface* const ei;
	/** @brief  The LogInterface to log messages */
	const LogInstruction li;

	/**
	 * @brief  Save pointer to state - should always exactly be called once
	 * @note   This should be given to the function as early as possible,
	 * 			so the ready should block as less as possible other actions
	 * @param  state: The state to be set
	 * 	- It should already contain opeations and the render-context-mask (RCTXM)
	 * 	- All other data should be complete on completin of the ready-function
	 * 	- Provide a nullptr ONLY to signalize that this wont have ANY ready-states for ANY operation
	 */
	virtual void setState(ReadyState* state) = 0;

	ReadyInstruction(std::vector<Identifier> ops, RenderContext* rctx, ExecutionInterface* ei, LogInstruction li)
		: ops(ops), rctx(rctx), ei(ei), li(li) {}
	virtual ~ReadyInstruction() {}

};

//
// UPSTREAM (READY)
//

class ReadyState {
public:
	ReadyState() {}
	virtual ~ReadyState() {}

	/** @brief The operations this state is valid for
	 * (Pointer is valid as long the object won't be modified/freed) */
	virtual const std::vector<Identifier>* getOperations() const = 0;

	/** @brief The mask of RenderContexts this state is valid for
	 * (Pointer is valid as long the object won't be modified/freed) */
	virtual const RenderContextMask* getContextMask() const = 0;

	/** @brief Memory the object takes up (in bytes) */
	virtual size_t size() const = 0;

	/** @brief Clone the object */
	virtual ReadyState* clone() const = 0;
};

//
// DISCOVERY
//

struct UserLabel {
	const char* name = nullptr;
	const char* description = nullptr;
};

/**
 * @brief  Interface to be returned by a module's DiscoveryFunction,
 * 		used to find contents of a certain torasu-module,
 * 		like information, factories and such.
 */
class DiscoveryInterface {
public:
	struct FactoryIndex {
		/** @brief  Pointer to array of DataResourceFactory,
		  * with length of dataFactoryCount */
		const DataResourceFactory* const* dataFactoryIndex;
		size_t dataFactoryCount;
		/** @brief  Pointer to array of ElementFactory,
		  * with length of elementFactoryCount */
		const ElementFactory* const* elementFactoryIndex;
		size_t elementFactoryCount;
	};

	/** @retval Index of Factories for types this module provides */
	virtual const FactoryIndex* getFactoryIndex() const = 0;
	/** @retval Label for this Module */
	virtual UserLabel getLabel() const = 0;

};

/**
 * @brief  Base class for all factories
 */
class FactoryBase {
public:
	/** @retval Type-Identifier of the Object to be created */
	virtual Identifier getType() const = 0;
	/** @retval Label for user to see on object that is created in this Factory  */
	virtual UserLabel getLabel() const = 0;
};

/**
 * @brief  Factory for creating DataResources from a data-dump
 */
class DataResourceFactory : public FactoryBase {
public:
	/**
	 * @brief  Create a DataResource-instance
	 * @param  dump: The dump previosly created by via DataResource.dumpResource()
	 * @retval The created DataResource (managed by caller)
	 */
	virtual DataResource* create(const DataDump* dump) const = 0;
};

/**
 * @brief  Factory for creating Elements from data and/or other Elements
 */
class ElementFactory : public FactoryBase {
public:
	/**
	 * @brief  Create an Element-instance
	 * @param  data: Double-pointer to Element-data, will be set to nullptr when consumed
	 * @param  elements: Element map containing elements to be set
	 * @retval The created Element (managed by caller)
	 */
	virtual Element* create(DataResource** data, const ElementMap& elements) const = 0;

	/** @brief  Describes an Element-slot */
	struct SlotDescriptor {
		/** @brief  Key of Slot, used to set in e.g. setElement(key, elem) */
		Identifier id;
		/** @brief  Label of the Slot for the user */
		UserLabel label;
		/** @brief  true if the slot may be left empty,
		  * false if it always has to be occupied with an Element */
		bool optional;
		/** @brief  true if it accepts any kind of Renderables,
		  * false if it reqires a special type of Element */
		bool renderable;
	};

	/** @brief  Index of all slots in the current Element */
	struct SlotIndex {
		/** @brief  Pointer to array of SlotDescriptor,
		  * with length of slotCount */
		const SlotDescriptor* slotIndex;
		const size_t slotCount;
	};

	/** @retval  Index of all slots in the current Element */
	virtual SlotIndex getSlotIndex() const = 0;
};

} /* namespace torasu */

#endif // CORE_INCLUDE_TORASU_TORASU_HPP_
