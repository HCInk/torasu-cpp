#include <stdexcept>

#include <torasu/torasu.hpp>

namespace torasu {

static class : public torasu::DataResourceFactory {
public:
	Identifier getType() const override {
		return "T::DRMU";
	}

	UserLabel getLabel() const override {
		return {
		name: "Unknown-Mask"
			,
		description: "Mask which indicates that matches cant be verfied for any item"
			" - nither positively or negatively"
		};
	}

	DataResource* create(const DataDump* dump) const override {
		return new DataResourceMask::DataResourceMaskUnknown();
	}
} MASK_UNKNOWN_FACTORY;

static class : public torasu::DataResourceFactory {
public:
	Identifier getType() const override {
		return "T::DRMS";
	}

	UserLabel getLabel() const override {
		return {
		name: "Single-Mask"
			,
		description: "Mask which only matches only one exact value"
		};
	}

	DataResource* create(const DataDump* dump) const override {
		// TODO Nested DPs required for this to work
		throw std::runtime_error("MASK_SINGLE_FACTORY-create not implemented yet");
	}
} MASK_SINGLE_FACTORY;

} // namespace torasu

namespace {

static torasu::DataResourceFactory* dataFactories[] = {
	&torasu::MASK_UNKNOWN_FACTORY,
	&torasu::MASK_SINGLE_FACTORY
};

static torasu::DiscoveryInterface::FactoryIndex factoryIndex = {
	// Data Factories
dataFactoryIndex:
	dataFactories,
dataFactoryCount:
	sizeof(dataFactories)/sizeof(torasu::DataResourceFactory),
	// Element Factories
elementFactoryIndex:
	nullptr,
	elementFactoryCount: 0,
};

static class : public torasu::DiscoveryInterface {

	const FactoryIndex* getFactoryIndex() const override {
		return &factoryIndex;
	}

	torasu::UserLabel getLabel() const override {
		return {
		name: "TORASU-Core"
			,
		description: "Core components needed for torasu function properly"
		};
	}

} DISCOVERY;

} // namespace

extern "C" const torasu::DiscoveryInterface* TORASU_DISCOVERY_torasu_core() {
	return &DISCOVERY;
}
