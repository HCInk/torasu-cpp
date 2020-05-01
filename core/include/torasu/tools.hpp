#ifndef CORE_INCLUDE_TORASU_TOOLS_HPP_
#define CORE_INCLUDE_TORASU_TOOLS_HPP_

#include <string>
#include <map>
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

	T* getResult() {
		return result;
	}

	ResultSegmentStatus getStatus() {
		return rs->getStatus();
	}
};

template<class T> CastedRenderSegmentResult<T>* findResult(RenderResult* rr, std::string& key) {

	std::map<std::string, ResultSegment*>* results = rr->getResults();
	std::map<std::string, ResultSegment*>::iterator found = results->find(key);

	if (found != results->end()) {

		return new CastedRenderSegmentResult<T>(found->second);

	} else {
		return NULL;
	}

}

} // namespace torasu::tools

#endif // CORE_INCLUDE_TORASU_TOOLS_HPP_
