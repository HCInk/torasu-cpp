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

// Error if a property has been provided, but hasn't been removed from the requests
#define TORASU_CHECK_FALSE_EJECTION

// Error if a render is enqueued with a null render context
#define TORASU_CHECK_RENDER_NULL_RCTX


int TORASU_check_core();

namespace torasu {

// INTERFACES
class ExecutionInterface;

// DATA
class DataDump;
class DataResource;

// TREE
class Element;
class Renderable;

// DOWNSTREAM
class RenderInstruction;
typedef std::map<std::string, DataResource*> RenderContext; // TODO "Real" RenderContext
class ResultFormatSettings;
class ResultSegmentSettings;
typedef std::vector<ResultSegmentSettings*> ResultSettings;

// UPSTREAM
class RenderResult;
class ResultSegment;

//
// INTERFACES
//

class ExecutionInterface {
public:
	virtual ~ExecutionInterface() {}
	virtual uint64_t enqueueRender(Renderable* rend, RenderContext* rctx, ResultSettings* rs, int64_t prio) = 0;
	virtual RenderResult* fetchRenderResult(uint64_t renderId) = 0;
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
};

//
// TREE
//

typedef std::map<std::string, Element*> ElementMap;

class Element {
public:
	Element() {
	}
	virtual ~Element() {
	}

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
	Renderable() {
	}
	virtual ~Renderable() {
	}

	virtual RenderResult* render(RenderInstruction* ri) = 0;
};

//
// DOWNSTREAM
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
// Upstream
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
	DataResource* result;
	bool freeResult;
public:

	/**
	 * @brief  Creates a ResultSegment (only status, without content)
	 * @note  This constructor should only be used if a result wasn't generated
	 * @param  status: Calculation-status of the segment
	 */
	explicit inline ResultSegment(ResultSegmentStatus status) {
		this->status = status;
		this->result = NULL;
		this->freeResult = false;
	}

	/**
	 * @brief  Creates a ResultSegment
	 * @param  status: Calculation-status of the segment
	 * @param  result: The result of the calculation of the segment
	 * @param  freeResult: Flag to destruct the DataResource of the result (true=will destruct)
	 */
	inline ResultSegment(ResultSegmentStatus status, DataResource* result, bool freeResult) {
		this->status = status;
		this->result = result;
		this->freeResult = freeResult;
	}

	~ResultSegment() {
		if (freeResult) {
			delete result;
		}
	}

	inline ResultSegmentStatus const getStatus() {
		return status;
	}

	inline DataResource* const getResult() {
		return result;
	}

	inline bool const canFreeResult() {
		return freeResult;
	}

	/**
	 * @brief  Ejects result to take control of manual memory management
	 * @note   Only available if canFreeResult() = false
	 * @retval The pointer to the result that has been ejected
	 */
	inline DataResource* const ejectResult() {
#ifdef TORASU_CHECK_FALSE_EJECTION
		if (!freeResult) {
			throw std::runtime_error("Ejected result that wasn't ejectable - canFreeResult() = false");
		}
#endif
		freeResult = false;
		return result;
	}

};

class RenderResult {
private:
	ResultStatus status;
	std::map<std::string, ResultSegment*>* results;
public:
	explicit inline RenderResult(ResultStatus status) {
		this->status = status;
		this->results = NULL;
	}

	inline RenderResult(ResultStatus status,
						std::map<std::string, ResultSegment*>* results) {
		this->status = status;
		this->results = results;
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

} /* namespace torasu */

#endif // CORE_INCLUDE_TORASU_TORASU_HPP_
