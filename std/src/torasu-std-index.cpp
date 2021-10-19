#include <torasu/torasu.hpp>

#include <torasu/std/torasu_full.hpp>

namespace {

static const torasu::DataResourceFactory* dataFactories[] = {
	torasu::tstd::Dnum::FACTORY,
	torasu::tstd::Dstring::FACTORY,
	// TODO Implement factories for the other types
};
static const torasu::ElementFactory* elementFactories[] = {
	torasu::tstd::Rnum::FACTORY,
	torasu::tstd::Rstring::FACTORY,
	torasu::tstd::Rmultiply::FACTORY,
	// TODO Implement factories for the other elements
};

static torasu::DiscoveryInterface::FactoryIndex factoryIndex = {
	// Data Factories
dataFactoryIndex:
	dataFactories,
dataFactoryCount:
	sizeof(dataFactories)/sizeof(torasu::DataResourceFactory),
	// Element Factories
elementFactoryIndex:
	elementFactories,
elementFactoryCount:
	sizeof(elementFactories)/sizeof(torasu::ElementFactory),
};

static class : public torasu::DiscoveryInterface {

	const FactoryIndex* getFactoryIndex() const override {
		return &factoryIndex;
	}

	torasu::UserLabel getLabel() const override {
		return {
		name: "TORASU-Standard"
			,
		description: "Standardised items for torasu to be used in various situations"
		};
	}

} DISCOVERY;

} // namespace

extern "C" const torasu::DiscoveryInterface* TORASU_DISCOVERY_torasu_std() {
	return &DISCOVERY;
}
