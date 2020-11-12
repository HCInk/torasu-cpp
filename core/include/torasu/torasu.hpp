/*
 * torasu.h
 *
 *  Created on: Mar 7, 2020
 */

#ifndef CORE_INCLUDE_TORASU_TORASU_HPP_
#define CORE_INCLUDE_TORASU_TORASU_HPP_

#include <string>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stdexcept>
#include <functional>
#include <mutex>
#include <chrono>

// Error if a property has been provided, but hasn't been removed from the requests
#define TORASU_CHECK_FALSE_EJECTION

// Error if a render is enqueued with a null render context
#define TORASU_CHECK_RENDER_NULL_RCTX

// Error if objects which support manual initialisation via a seperate functions are initialized twice
#define TORASU_CHECK_DOUBLE_INIT

int TORASU_check_core();

namespace torasu {

// HELPER (INTERFACES)
typedef uint64_t LockId;

// INTERFACES
class ExecutionInterface;

// DATA
class DataDump;
class DataResource;
class DataResourceHolder;

// TREE
class Element;
class Renderable;

// HELPER (TREE)
class ElementExecutionOpaque;

// DOWNSTREAM (RENDER)
class RenderInstruction;
typedef std::map<std::string, DataResource*> RenderContext; // TODO "Real" RenderContext
class ResultFormatSettings;
class ResultSegmentSettings;
typedef std::vector<ResultSegmentSettings*> ResultSettings;

// UPSTREAM (RENDER)
class RenderResult;
class ResultSegment;

// HELPER (READY)
typedef uint64_t ReadyObject;
typedef std::vector<ReadyObject> ReadyObjects;

// DOWNSTREAM (READY)
class ReadyRequest;
class ReadyInstruction;
class UnreadyInstruction;

// UPSTREAM (READY)
class ObjectReadyResult;
typedef std::vector<ObjectReadyResult> ElementReadyResult;

//
// INTERFACES
//

class ExecutionInterface {
public:
	virtual ~ExecutionInterface() {}
	virtual uint64_t enqueueRender(Renderable* rend, RenderContext* rctx, ResultSettings* rs, int64_t prio) = 0;
	virtual RenderResult* fetchRenderResult(uint64_t renderId) = 0;
	virtual void lock(LockId lockId=0) = 0;
	virtual void unlock(LockId lockId=0) = 0;
};

//
// DATA
//

typedef int (*two_num_operation)(int, int);

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

	inline DDDataPointer const getData() {
		return data;
	}

	inline int const getSize() {
		return size;
	}

	inline bool isJson() {
		return json;
	}

	inline DataDump* newReference() {
		return new DataDump(data, size, nullptr, json);
	}
};

class DataResource {
public:
	DataResource() {}
	virtual ~DataResource() {}

	virtual std::string getIdent() = 0;
	virtual DataDump* dumpResource() = 0;
	virtual DataResource* clone() = 0;
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
// TREE
//

class ElementExecutionOpaque {
public:
	ElementExecutionOpaque() {}
	virtual ~ElementExecutionOpaque() {}
};

typedef std::map<std::string, Element*> ElementMap;

class Element {
public:
	ElementExecutionOpaque* elementExecutionOpaque = nullptr;
	std::mutex elementExecutionOpaqueLock;

	Element() {}
	virtual ~Element() {
		if (elementExecutionOpaque != nullptr) delete elementExecutionOpaque;
	}

	virtual ReadyObjects* requestReady(const ReadyRequest& ri) = 0;
	virtual ElementReadyResult* ready(const ReadyInstruction& ri) = 0;
	virtual void unready(const UnreadyInstruction& uri) = 0;

	virtual std::string getType() = 0;
	virtual DataResource* getData() = 0;
	virtual ElementMap getElements() = 0;

	virtual void setData(DataResource* data,
						 ElementMap elements) = 0;
	virtual void setData(DataResource* data) = 0;
	virtual void setElement(std::string key, Element* elem) = 0;

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

public:
	inline RenderInstruction(RenderContext* rctx, ResultSettings* rs, ExecutionInterface* ei)
		: rctx(rctx), rs(rs), ei(ei) {}

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
};

class ResultSegmentSettings {
private:
	std::string pipeline;
	std::string key;
	ResultFormatSettings* rfs;
public:
	inline ResultSegmentSettings(std::string pipeline, std::string key,
								 ResultFormatSettings* rfs) {
		this->pipeline = pipeline;
		this->key = key;
		this->rfs = rfs;
	}

	~ResultSegmentSettings() {

	}

	inline std::string const getPipeline() {
		return pipeline;
	}

	inline std::string const getKey() {
		return key;
	}

	inline ResultFormatSettings* const getResultFormatSettings() {
		return rfs;
	}
};

#define TORASU_FORMAT_PREFIX "F#"

class ResultFormatSettings : public virtual DataResource {
private:
	std::string ident;
public:
	ResultFormatSettings(std::string dataType)
		: ident(TORASU_FORMAT_PREFIX + dataType) {}
	virtual ~ResultFormatSettings() {}

	std::string getIdent() override {
		return ident;
	}

	virtual std::set<std::string> const getTags() {
		return std::set<std::string>();
	}

};

//
// Upstream (RENDER)
//

enum ResultStatus {
	/**Request wasnt processed at all because it was received as malformed
	 */
	ResultStatus_MALFORMED = -3,

	/**Some segments might not be present as requested,
	 * refer to the individual segment-status
	 */
	ResultStatus_PARTIAL_ERROR = -2,

	/**An internal error or undefined behavior
	 * occurred during the execution
	 */
	ResultStatus_INTERNAL_ERROR = -1,

	/**Everything went as expected
	 */
	ResultStatus_OK = 0,

	/**Warnings were thrown while running
	 */
	ResultStatus_OK_WARN = 2
};

enum ResultSegmentStatus {
	/**The given format is not available
	 */
	ResultSegmentStatus_INVALID_FORMAT = -5,

	/**The given segment was not provided
	 * @note This is reserved for wrappers, to signalize,
	 * 	that the Segment is completely absent from the given ResultSettings.
	 * 	This should never be returned in a first-party generated ResultSegment,
	 * 	if you want to signaize, that a segment is not supported,
	 * 	then use ResultSegmentStatus_INVALID_SEGMENT serves that purpose
	 *
	 */
	ResultSegmentStatus_NON_EXISTENT = -4,

	/**The given segment is invalid
	 */
	ResultSegmentStatus_INVALID_SEGMENT = -3,

	/**An internal error or undefined behavior
	 * occurred during the execution
	 */
	ResultSegmentStatus_INTERNAL_ERROR = -1,

	/**Everything went as expected
	 */
	ResultSegmentStatus_OK = 0,

	/**Warnings were thrown while running
	 */
	ResultSegmentStatus_OK_WARN = 2
};

class ResultSegment {
private:
	ResultSegmentStatus status;
	DataResourceHolder result;

public:

	/**
	 * @brief  Creates a ResultSegment (only status, without content)
	 * @note  This constructor should only be used if a result wasn't generated
	 * @param  status: Calculation-status of the segment
	 */
	explicit inline ResultSegment(ResultSegmentStatus status)
		: status(status), result() {}

	/**
	 * @brief  Creates a ResultSegment
	 * @param  status: Calculation-status of the segment
	 * @param  result: The result of the calculation of the segment
	 * @param  freeResult: Flag to destruct the DataResource of the result (true=will destruct)
	 */
	inline ResultSegment(ResultSegmentStatus status, DataResource* result, bool freeResult)
		: status(status), result(result, freeResult) {}

	~ResultSegment() {}

	inline ResultSegmentStatus const getStatus() {
		return status;
	}

	inline DataResource* const getResult() {
		return result.get();
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

};

class RenderResult {
private:
	ResultStatus status;
	std::map<std::string, ResultSegment*>* results;
public:
	std::chrono::_V2::system_clock::time_point creation;
	explicit inline RenderResult(ResultStatus status) {
		this->status = status;
		this->results = NULL;
		this->creation = std::chrono::high_resolution_clock::now(); // XXX for measurement purposes
	}

	inline RenderResult(ResultStatus status,
						std::map<std::string, ResultSegment*>* results) {
		this->status = status;
		this->results = results;
		this->creation = std::chrono::high_resolution_clock::now(); // XXX for measurement purposes
	}

	~RenderResult() {
		if (results != NULL) {
			for (std::pair<std::string, ResultSegment*> res : *results) {
				delete res.second;
			}
			delete results;
		}
	}

	inline ResultStatus const getStatus() {
		return status;
	}

	inline std::map<std::string, ResultSegment*>* const getResults() {
		return results;
	}
};

//
// DOWNSTREAM (READY)
//

/**
 * @brief  Request to fetch which objects need to be made ready to run a desired task (ReadyObjects)
 */
class ReadyRequest {
private:
	std::vector<std::string> ops;
	RenderContext* rctx;
public:
	inline ReadyRequest(std::vector<std::string> ops, RenderContext* rctx)
		: ops(ops), rctx(rctx)  {}
	~ReadyRequest() {}

	inline std::vector<std::string>& getOpeations() {
		return ops;
	}

	inline RenderContext* getRenderContext() {
		return rctx;
	}
};

/**
 * @brief  Instruction to make an Element ready
 */
class ReadyInstruction {
private:
	ReadyObjects objects;
	ExecutionInterface* ei;

public:
	inline ReadyInstruction(ReadyObjects objects, ExecutionInterface* ei)
		: objects(objects), ei(ei) {}
	~ReadyInstruction() {}

	inline ReadyObjects& getObjects() {
		return objects;
	}

	inline ExecutionInterface* getExecutionInterface() {
		return ei;
	}
};

/**
 * @brief  Instruction to unready an Element
 */
class UnreadyInstruction {
private:
	ReadyObjects objects;

public:
	inline explicit UnreadyInstruction(ReadyObjects objects)
		: objects(objects) {}
	~UnreadyInstruction() {}

	inline ReadyObjects& getObjects() {
		return objects;
	}
};

//
// UPSTREAM (READY)
//

/**
 * @brief  Result of making a object inside an Element ready
 */
class ObjectReadyResult {
private:
	ReadyObject obj;

public:
	inline explicit ObjectReadyResult(ReadyObject obj)
		: obj(obj) {}
	~ObjectReadyResult();

	inline ReadyObject getObject() {
		return obj;
	}
};

} /* namespace torasu */

#endif // CORE_INCLUDE_TORASU_TORASU_HPP_
