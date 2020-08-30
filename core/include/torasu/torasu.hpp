/*
 * torasu.h
 *
 *  Created on: Mar 7, 2020
 */

#ifndef CORE_INCLUDE_TORASU_TORASU_HPP_
#define CORE_INCLUDE_TORASU_TORASU_HPP_

#ifdef _WIN32
#ifdef EXPORT_TORASUCORE
#define DLL_EXPORT __declspec(dllexport)
#pragma message ("123123132131")
#endif

#ifndef EXPORT_TORASUCORE

#define DLL_EXPORT __declspec(dllimport)
#pragma message ("4564565464")

#endif

#elif
#define DLL_EXPORT
#endif

#include <string>
#include <map>
#include <set>
#include <vector>
#include <utility>

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

class DLL_EXPORT ExecutionInterface {
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

/**
 * @brief Defines how the DD
 */
enum DDDataPointerType {
	/**unsigned char*
	 */
	DDDataPointerType_BINARY = 0,
	/**const char*
	 */
	DDDataPointerType_CSTR = 1,
	/**const char* (json conform)
	 */
	DDDataPointerType_JSON_CSTR = 2
};

class DLL_EXPORT DataDump {
// TODO coditional dereferencing
private:
	DDDataPointer data;
	int size;
	DDDataPointerType format;
	void (*freeFunc)(DataDump* data);
public:
	inline DataDump(DDDataPointer data, int size, DDDataPointerType format, void (*freeFunc)(DataDump* data)) {
		this->data = data;
		this->size = size;
		this->format = format;
		this->freeFunc = freeFunc;
	}

	~DataDump() {
		if (freeFunc != NULL) {
			freeFunc(this);
		}
	}

	inline DDDataPointer const getData() {
		return data;
	}

	inline int const getSize() {
		return size;
	}

	inline DDDataPointerType getFormat() {
		return format;
	}

	inline DataDump* newReference() {
		return new DataDump(data, size, format, freeFunc);
	}
};

class DLL_EXPORT DataResource {
public:
	DataResource() {}
	virtual ~DataResource() {}

	virtual std::string getIdent() = 0;
	virtual DataDump* getData() = 0;
};

//
// TREE
//

class DLL_EXPORT Element {
public:
	Element() {
	}
	virtual ~Element() {
	}

	virtual std::string getType() = 0;
	virtual DataResource* getData() = 0;
	virtual std::map<std::string, Element*> getElements() = 0;

	virtual void setData(DataResource* data,
						 std::map<std::string, Element*> elements) = 0;
	virtual void setData(DataResource* data) = 0;
	virtual void setElement(std::string key, Element* elem) = 0;

};

class DLL_EXPORT Renderable : public virtual Element {
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

class DLL_EXPORT RenderInstruction {
private:
	RenderContext* rctx;
	ResultSettings* rs;
	ExecutionInterface* ei;

public:
	inline RenderInstruction(RenderContext* rctx, ResultSettings* rs, ExecutionInterface* ei) {
		this->rctx = rctx;
		this->rs = rs;
		this->ei = ei;
	}

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

class DLL_EXPORT ResultSegmentSettings {
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

class DLL_EXPORT ResultFormatSettings {
private:
	std::string format;
	std::set<std::string>* tags;
	DataResource* data;
public:
	inline ResultFormatSettings(std::string format, std::set<std::string>* tags,
								DataResource* data) {
		this->format = format;
		this->tags = tags;
		this->data = data;
	}
	~ResultFormatSettings() {

	}

	inline std::string const getFormat() {
		return format;
	}

	inline std::set<std::string>* const getTags() {
		return tags;
	}

	inline DataResource* const getData() {
		return data;
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

class DLL_EXPORT ResultSegment {
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

};

class DLL_EXPORT RenderResult {
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
