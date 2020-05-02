#ifndef CORE_INCLUDE_TORASU_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_TOOLS_HPP_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

#include <torasu/torasu.hpp>

namespace torasu::tools {

//
// RenderTools
//

template<class T> class CastedRenderSegmentResult {
private:
	T* result;
	ResultSegment* rs;
public:
	explicit CastedRenderSegmentResult(ResultSegment* rs)  {
		this->rs = rs;

		if(T* casted = dynamic_cast<T*>(rs->getResult())) {
			result = casted;
		} else {
			std::ostringstream errMsg;
			errMsg << "Returned object is not of the expected type\""
				   << typeid(T).name()
				   << "\"!";
			throw std::logic_error(errMsg.str());
		}
	}

	~CastedRenderSegmentResult() {}

	inline T* getResult() {
		return result;
	}

	inline ResultSegmentStatus getStatus() {
		return rs->getStatus();
	}
};

template<class T> inline CastedRenderSegmentResult<T>* findResult(RenderResult* rr, const std::string& key) {

	std::map<std::string, ResultSegment*>::iterator found = rr->getResults()->find(key);
	if (found != rr->getResults()->end()) {
		return new CastedRenderSegmentResult<T>(found->second);
	} else {
		return NULL;
	}

}

template<class T> class RenderResultSegmentHandle {
private:
	std::string segKey;
public:
	explicit RenderResultSegmentHandle(std::string segKey) {
		this->segKey = segKey;
	}

	~RenderResultSegmentHandle() {}

	inline CastedRenderSegmentResult<T>* getFrom(RenderResult* rr) {
		return findResult<T>(rr, segKey);
	}

};

class RenderInstructionBuilder {
private:
	ResultSettings* rs = NULL;
	std::vector<ResultSegmentSettings*>* segments;

	void buildResultSettings();

	inline ResultSettings* getResultSetttings() {
		if (rs != NULL) {
			return rs;
		} else {
			buildResultSettings();
			return rs;
		}
	}

	int autoKeyIndex = 0;

	inline std::string getUnusedKey() {
		autoKeyIndex++;
		return std::to_string(autoKeyIndex-1);
	}

public:
	RenderInstructionBuilder();
	~RenderInstructionBuilder();

	inline void addSegment(const std::string& pipeline, const std::string& key, ResultFormatSettings* format) {
		segments->push_back(new ResultSegmentSettings(pipeline, key, format));
	}

	template<class T> inline RenderResultSegmentHandle<T> addSegmentWithHandle(const std::string& pipeline, ResultFormatSettings* format) {
		std::string segKey = getUnusedKey();
		addSegment(pipeline, segKey, format);
		return RenderResultSegmentHandle<T>(segKey);
	}

	inline RenderInstruction getInstruction(RenderContext* rctx) {
		return RenderInstruction(rctx, getResultSetttings());
	}
};


} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_TOOLS_HPP_
