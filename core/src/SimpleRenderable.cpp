#include "../include/torasu/SimpleRenderable.hpp"

#include <torasu/log_tools.hpp>

namespace torasu::tools {

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

torasu::ElementMap SimpleDataElement::getElements() {
	if (!acceptElements) {
		return torasu::ElementMap();
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

const torasu::OptElementSlot SimpleDataElement::setElement(std::string key, const ElementSlot* elem) {
	if (!acceptElements) {
		return nullptr;
	} else {
		throw std::logic_error("SimpleDataElement-impl-err: setElement(key, elem) is not defined,"
							   "even though elements are set to be accepted.");
	}
}

void SimpleDataElement::setData(DataResource* data,
								torasu::ElementMap elements) {
	if (acceptElements) {

		torasu::ElementMap previousElements = getElements();

		for (auto elemEntry : elements) {
			setElement(elemEntry.first, &elemEntry.second);
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

NoneReadyState::NoneReadyState(const std::vector<Identifier>& operations)
	: operations(new std::vector<Identifier>(operations)), rctxm(new RenderContextMask()) {}

NoneReadyState::NoneReadyState(const NoneReadyState& orig)
	: operations(new std::vector<Identifier>(*orig.operations)), rctxm(new RenderContextMask()) {}

NoneReadyState::~NoneReadyState() {
	delete operations;
	delete rctxm;
}

const std::vector<Identifier>* NoneReadyState::getOperations() const {
	return operations;
}

const RenderContextMask* NoneReadyState::getContextMask() const {
	return rctxm;
}

size_t NoneReadyState::size() const {
	return sizeof(NoneReadyState);
}

NoneReadyState* NoneReadyState::clone() const {
	return new NoneReadyState(*this);
}

ReadylessElement::ReadylessElement() {}
ReadylessElement::~ReadylessElement() {}

void ReadylessElement::ready(ReadyInstruction* ri) {
	ri->setState(nullptr);
}

SimpleRenderable::SimpleRenderable(bool acceptData, bool acceptElements)
	: SimpleDataElement(acceptData, acceptElements) {}

SimpleRenderable::~SimpleRenderable() {}

} // namespace torasu::tools