#include "../include/torasu/SimpleRenderable.hpp"

namespace torasu::tools {

NamedIdentElement::NamedIdentElement(std::string typeIdent)
	: typeIdent(typeIdent) {}

NamedIdentElement::~NamedIdentElement() {}

std::string NamedIdentElement::getType() {
	return typeIdent;
}

SimpleDataElement::SimpleDataElement(bool acceptData, bool acceptElements)
	: acceptData(acceptData), acceptElements(acceptElements) {}

SimpleDataElement::~SimpleDataElement() {}

DataResource* SimpleDataElement::getData() {
	if (!acceptData) {
		return NULL;
	} else {
		throw std::logic_error("SimpleDataElement-impl-err: getData(data) is not defined,"
							   "even though data is set to be accepted");
	}
}

std::map<std::string, Element*> SimpleDataElement::getElements() {
	if (!acceptElements) {
		return std::map<std::string, Element*>();
	} else {
		throw std::logic_error("SimpleDataElement-impl-err: getElements(data) is not defined,"
							   "even though elements are set to be accepted.");
	}
}

void SimpleDataElement::setData(DataResource* data) {
	if (!acceptData) {
		throw std::invalid_argument("This element does not accept any data!");
	} else {
		throw std::logic_error("SimpleDataElement-impl-err: setData(data) is not defined,"
							   "even though data is set to be accepted.");
	}
}

void SimpleDataElement::setElement(std::string key, Element* elem) {
	if (!acceptElements) {
		throw std::invalid_argument("This element does not accept any elements!");
	} else {
		throw std::logic_error("SimpleDataElement-impl-err: setElement(key, elem) is not defined,"
							   "even though elements are set to be accepted.");
	}
}

void SimpleDataElement::setData(DataResource* data,
								std::map<std::string, Element*> elements) {
	if (acceptElements) {

		std::map<std::string, Element*> previousElements = getElements();

		for (auto elemEntry : elements) {
			setElement(elemEntry.first, elemEntry.second);
			previousElements.erase(elemEntry.first);
		}

		for (auto toRemoveElement : previousElements ) {
			setElement(toRemoveElement.first, NULL);
		}

	} else if (elements.size() > 0) {
		throw std::invalid_argument("Elements were were added, but this element does not accept any elements!");
	}

	setData(data);

}


IndividualizedSegnentRenderable::IndividualizedSegnentRenderable() {}
IndividualizedSegnentRenderable::~IndividualizedSegnentRenderable() {}

RenderResult* IndividualizedSegnentRenderable::render(RenderInstruction* ri) {

	auto rs = ri->getResultSettings();

	std::map<std::string, ResultSegment*>* results = new std::map<std::string, ResultSegment*>();

	bool hasError = false;
	bool hasWarn = false;

	for (ResultSegmentSettings* rss : *rs) {

		ResultSegment* rseg;

		try {
			rseg = renderSegment(rss, ri);
		} catch (const std::exception& ex) {
			std::cerr << "SimpleRenderable error: " << ex.what() << std::endl;
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


NoPropRenderable::NoPropRenderable() {}
NoPropRenderable::~NoPropRenderable() {}

RenderableProperties* NoPropRenderable::getProperties(PropertyInstruction* pi) {
	return new RenderableProperties();
}

SimpleRenderable::SimpleRenderable(std::string typeIdent, bool acceptData, bool acceptElements)
	: NamedIdentElement(typeIdent),
	  SimpleDataElement(acceptData, acceptElements) {}

SimpleRenderable::~SimpleRenderable() {}

} // namespace torasu::tools