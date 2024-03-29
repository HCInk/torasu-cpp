#include "../include/torasu/std/Rmix_pipelines.hpp"

#include <torasu/render_tools.hpp>

namespace torasu::tstd {

//
//	Dmix_pipelines_conf
//

void Dmix_pipelines_conf::load() {
	// TODO Implement Dmix_pipelines_conf::load
	throw std::logic_error("No loading of Dmix_pipelines_conf supported yet!");
}

torasu::json Dmix_pipelines_conf::makeJson() {
	torasu::json json;

	torasu::json elemMap;

	for (auto entry : mappingsById) elemMap[entry.first] = entry.second->pl;

	json["elems"] = elemMap;

	return json;
}

torasu::Identifier Dmix_pipelines_conf::getType() const {
	return "STD::DMIX_PIPELINES_CONF";
}

Dmix_pipelines_conf* Dmix_pipelines_conf::clone() const {
	return new Dmix_pipelines_conf(*this);
}

void Dmix_pipelines_conf::applyMappings(std::vector<PipelineMappingUnmanaged> newMappings) {

	// Save previous mappings
	std::set<PipelineMapping*> oldMappings;
	for (auto& oldMapping : mappingsById) oldMappings.insert(oldMapping.second);

	// Apply new mappings
	for (auto& newMapping : newMappings) {

		auto foundIdMapping = mappingsById.find(newMapping.id);
		PipelineMapping* mapping;
		if (foundIdMapping != mappingsById.end()) {
			mapping = foundIdMapping->second;
			oldMappings.erase(mapping);

			// Update renderable (if given)
			if (newMapping.rnd.get() != nullptr)
				mapping->rnd = newMapping.rnd;

			// Remap pipeline if it changed
			if (mapping->pl != newMapping.pl) {

				// Remove previous pl-mapping, if it has not been remapped yet
				if (!mapping->pl.empty()) {
					auto foundPlMapping = mappingsByPl.find(mapping->pl);
					if (foundPlMapping->second == mapping) {
						mappingsByPl.erase(foundPlMapping);
					}
				}

				// Set new mapping
				mappingsByPl[newMapping.pl] = mapping;

			}

		} else {
			mapping = new PipelineMapping(newMapping);
			mappingsById[mapping->id] = mapping;
			mappingsByPl[mapping->pl] = mapping;
		}

	}

	// Remove old mappings
	for (auto toRemove : oldMappings) {
		// Should always erase one
		mappingsById.erase(toRemove->id);
		auto foundPlMapping = mappingsByPl.find(toRemove->pl);
		// Should erase one if pipeline has not been reused by other id
		if (foundPlMapping->second == toRemove) mappingsByPl.erase(foundPlMapping);
		// Free mapping
		delete toRemove;
	}

}

void Dmix_pipelines_conf::updateMapping(size_t id, const torasu::RenderableSlot& rnd) {
	auto foundMapping = mappingsById.find(id);

	if (foundMapping != mappingsById.end()) {
		foundMapping->second->rnd = rnd;
	} else {
		auto* mapping = new PipelineMapping{
			id,
			"", // pl.empty(): Signalise that the pipeline is not known yet
			rnd
		};
		mappingsById[id] = mapping;

	}
}

void Dmix_pipelines_conf::importMappings(const Dmix_pipelines_conf* newMappings) {
	std::vector<Dmix_pipelines_conf::PipelineMappingUnmanaged> mappings;
	for (auto entry : newMappings->mappingsById) mappings.push_back({
		entry.second->id,
		entry.second->pl,
		torasu::RenderableSlot()
	});
	applyMappings(mappings);
}

Dmix_pipelines_conf::~Dmix_pipelines_conf() {
	for (auto& mapping : mappingsById) delete mapping.second;
}

//
//	Rmix_pipelines
//

Rmix_pipelines::Rmix_pipelines(torasu::RenderableSlot def, std::initializer_list<MixEntry> mixes)
	: SimpleRenderable(true, true), defRnd(def) {

	std::vector<Dmix_pipelines_conf::PipelineMappingUnmanaged> entries;
	size_t id = 0;
	for (auto& entry : mixes) {
		entries.push_back({
			id,
			entry.pl,
			entry.rnd
		});
		id++;
	}

	conf.applyMappings(entries);
}

Rmix_pipelines::~Rmix_pipelines() {}

Identifier Rmix_pipelines::getType() {
	return "STD::RMIX_PIPELINES";
}

torasu::RenderResult* Rmix_pipelines::render(torasu::RenderInstruction* ri) {
	torasu::tools::RenderHelper rh(ri);
	auto pipeline = ri->getResultSettings()->getPipeline();

	auto found = conf.mappingsByPl.find(pipeline.str);

	Renderable* rnd;

	if (found != conf.mappingsByPl.end()) {
		rnd = found->second->rnd.get();

		// TODO make sanity-check optional
		if (rnd == nullptr)
			throw std::runtime_error("Renderable-mapping found for "
									 "PL=" + pipeline + " (ID" + std::to_string(found->second->id) + ") "
									 "contains no Renderable!");
	} else {
		// If default-renderable is null, then render only mapped segments,
		// otherwise use defRnd as fallback
		if (defRnd.get() != nullptr) {
			rnd = defRnd.get();
		} else {
			return new torasu::RenderResult(RenderResultStatus_INVALID_SEGMENT);
		}
	}

	return rh.runRender(rnd, ri->getResultSettings());
}

#define DEFAULT_KEY "def"
#define ELEM_KEY_PFX "e"
#define ELEM_KEY_PFX_LEN 1

torasu::ElementMap Rmix_pipelines::getElements() {
	torasu::ElementMap elems;
	elems[DEFAULT_KEY] = defRnd;

	for (auto& entry : conf.mappingsById)
		elems[ELEM_KEY_PFX + std::to_string(entry.first)] = entry.second->rnd;

	return elems;
}

const torasu::OptElementSlot Rmix_pipelines::setElement(std::string key, const ElementSlot* elem) {
	if (key == DEFAULT_KEY) return torasu::tools::trySetRenderableSlot(&defRnd, elem);

	if (key.substr(0, ELEM_KEY_PFX_LEN) != ELEM_KEY_PFX) return nullptr;
	size_t id;
	try {
		id = std::stoul(key.substr(ELEM_KEY_PFX_LEN));
	} catch (const std::exception& ex) {
		return nullptr;
	}

	if (elem == nullptr) {
		conf.updateMapping(id, nullptr);
	}

	if (auto* rnd = dynamic_cast<Renderable*>(elem->get())) {
		conf.updateMapping(id, RenderableSlot(rnd, elem->isOwned()));
	}

	auto found = conf.mappingsById.find(id);
	return (found != conf.mappingsById.end()) ? found->second->rnd.asElementSlot() : nullptr;
}

torasu::DataResource* Rmix_pipelines::getData() {
	return &conf;
}

void Rmix_pipelines::setData(torasu::DataResource* data) {
	if (auto* newConf = dynamic_cast<torasu::tstd::Dmix_pipelines_conf*>(data)) {
		conf.importMappings(newConf);
	} else {
		throw std::invalid_argument("Provided data is not Dmix_pipelines_conf!");
	}
}



} // namespace torasu::tstd