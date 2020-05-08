#include "../include/torasu/SimpleRenderable.hpp"

namespace torasu::tools {

SimpleRenderable::SimpleRenderable(std::string typeIdent, bool acceptData, bool acceptElements) {
	this->typeIdent = typeIdent;
	this->acceptData = acceptData;
	this->acceptElements = acceptElements;
}

SimpleRenderable::~SimpleRenderable() {

}

std::string SimpleRenderable::getType() {
	return typeIdent;
}

DataResource* SimpleRenderable::getData() {
	if (!acceptData) {
		return NULL;
	} else {
		throw std::logic_error("SimpleRenderable-impl-err: getData(data) is not defined,"
							   "even though data is set to be accepted");
	}
}

std::map<std::string, Element*> SimpleRenderable::getElements() {
	if (!acceptElements) {
		return std::map<std::string, Element*>();
	} else {
		throw std::logic_error("SimpleRenderable-impl-err: getElements(data) is not defined,"
							   "even though elements are set to be accepted.");
	}
}

void SimpleRenderable::resetElements() {
	throw std::logic_error("SimpleRenderable-impl-err: Elements are getting set, but resetElements() is not implemented!\n"
						   "- Make sure resetElements() is implemented, when the element accepts elements "
						   "or implement setData(data, elements) yourself and remove old elements there.");
}

void SimpleRenderable::setData(DataResource* data) {
	if (!acceptData) {
		throw std::invalid_argument("This element does not accept any data!");
	} else {
		throw std::logic_error("SimpleRenderable-impl-err: setData(data) is not defined,"
							   "even though data is set to be accepted.");
	}
}

void SimpleRenderable::setElement(std::string key, Element* elem) {
	if (!acceptElements) {
		throw std::invalid_argument("This element does not accept any elements!");
	} else {
		throw std::logic_error("SimpleRenderable-impl-err: setElement(key, elem) is not defined,"
							   "even though elements are set to be accepted.");
	}
}

void SimpleRenderable::setData(DataResource* data,
							   std::map<std::string, Element*> elements) {
	if (acceptElements) {
		resetElements();

		for (auto elemEntry : elements) {
			setElement(elemEntry.first, elemEntry.second);
		}

	} else if (elements.size() > 0) {
		throw std::invalid_argument("Elements were were added, but this element does not accept any elements!");
	}

	setData(data);
	
}

RenderResult* SimpleRenderable::render(RenderInstruction* ri) {

	auto rs = ri->getResultSettings();

	std::map<std::string, ResultSegment*>* results = new std::map<std::string, ResultSegment*>();

	bool hasError = false;
	bool hasWarn = false;

	for (ResultSegmentSettings* rss : *rs) {

		ResultSegment* rseg;

		try {
			rseg = renderSegment(rss, ri);
		} catch (const std::exception& ex) {
			std::cerr << "SimpleRenderable" << std::endl;
			(*results)[rss->getKey()] = new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
			continue;
		}

		if (rseg != NULL) {

			ResultSegmentStatus status = rseg->getStatus();
			if (status < 0) {
				hasError = true;
			} else if (status == ResultSegmentStatus_OK_WARN) {
				hasWarn = true;
			}

			(*results)[rss->getKey()] = rseg;
			continue;

		} else {
			(*results)[rss->getKey()] = new ResultSegment(ResultSegmentStatus_INTERNAL_ERROR);
			continue;
		}

	}

	ResultStatus summarizedStatus;

	if (hasError) {
		summarizedStatus = ResultStatus::ResultStatus_PARTIAL_ERROR;
	} else if (hasWarn) {
		summarizedStatus = ResultStatus::ResultStatus_OK_WARN;
	} else {
		summarizedStatus = ResultStatus::ResultStatus_OK;
	}

	return new RenderResult(summarizedStatus, results);
}

} // namespace torasu::tools