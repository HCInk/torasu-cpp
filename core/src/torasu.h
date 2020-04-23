/*
 * torasu.h
 *
 *  Created on: Mar 7, 2020
 */

#ifndef TORASU_H_
#define TORASU_H_

#include <string>
#include <map>
#include <set>
#include <vector>

namespace torasu {

// TREE
class Element;
class Renderable;

// DOWNSTREAM
class RenderInstruction;
typedef std::map<std::string, Element*> RenderContext; // TODO "Real" RenderContext
class ResultFormatSettings;
class ResultSegmentSettings;
typedef std::vector<ResultSegmentSettings*> ResultSettings;

// UPSTREAM
class RenderResult;
class ResultSegment;

// DATA
class DataDump;
class DataResource;

//
// TREE
//

class Element {
public:
	Element() {
	}
	virtual ~Element() {
	}

	virtual std::string getType() = 0;
	virtual DataResource* geData() = 0;
	virtual std::map<std::string, Element*> getElements() = 0;

	virtual void setData(DataResource* data,
			std::map<std::string, Element*> elements) = 0;
	virtual void setData(DataResource* data) = 0;
	virtual void setElement(std::string key, Element* elem) = 0;

};

class Renderable : public Element {
public:
	Renderable() {
	}
	virtual ~Renderable() {
	}

	virtual RenderResult render(RenderInstruction* ri) = 0;
};

//
// DOWNSTREAM
//

class RenderInstruction {
private:
	RenderContext* rctx;
	ResultSettings* rs;
public:
	inline RenderInstruction(RenderContext* rctx, ResultSettings* rs) {
		this->rctx = rctx;
		this->rs = rs;
	}

	~RenderInstruction() {

	}

	inline RenderContext* const getRenderContext() {
		return rctx;
	}

	inline ResultSettings* const getResultSettings() {
		return rs;
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

class ResultFormatSettings {
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
	/**The given segment is invalid
	 */
	ResultSegmentStatus_INVALID_SEGMENT = -4,
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

class RenderResult {
private:
	ResultStatus status;
	std::map<std::string, ResultSegment*>* results;
public:
	inline RenderResult(ResultStatus status,
			std::map<std::string, ResultSegment*>* results) {
		this->status = status;
		this->results = results;
	}

	~RenderResult() {

	}

	inline ResultStatus const getStatus() {
		return status;
	}

	inline std::map<std::string, ResultSegment*>* const getResults() {
		return results;
	}
};

class ResultSegment {
private:
	ResultSegmentStatus status;
	DataResource* result;
public:
	inline ResultSegment(ResultSegmentStatus status, DataResource* result) {
		this->status = status;
		this->result = result;
	}

	~ResultSegment() {

	}

	inline ResultSegmentStatus const getStatus() {
		return status;
	}

	inline DataResource* const getResult() {
		return result;
	}

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

class DataDump {
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

class DataResource {
public:
	DataResource() {}

	virtual ~DataResource() {}

	virtual std::string getIdent() = 0;
	virtual DataDump getData() = 0;
};

} /* namespace torasu */

#endif /* TORASU_H_ */
