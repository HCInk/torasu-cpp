#ifndef STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_
#define STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_

#include <map>
#include <vector>
#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/slot_tools.hpp>

namespace torasu::tstd {

class Rmix_pipelines;

class Dmix_pipelines_conf : public torasu::DataPackable {

private:
	// For configuration
	struct PipelineMappingUnmanaged {
		size_t id;
		std::string pl;
		torasu::RenderableSlot rnd;
	};

	// For configuration
	struct PipelineMapping {
		size_t id;
		std::string pl;
		torasu::tools::ManagedRenderableSlot rnd; // Will not be exported/imported

		explicit inline PipelineMapping(size_t id, std::string pl, torasu::RenderableSlot rnd)
			: id(id), pl(pl), rnd(rnd) {}

		explicit inline PipelineMapping(const PipelineMappingUnmanaged& um)
			: id(um.id), pl(um.pl), rnd(um.rnd) {}
	};

	// mappings (managed by applyMappings(...) and updateMapping(...))
	std::map<size_t, PipelineMapping*> mappingsById;
	std::map<std::string, PipelineMapping*> mappingsByPl;

	void applyMappings(std::vector<PipelineMappingUnmanaged> newMappings);
	void updateMapping(size_t id, const torasu::RenderableSlot& rnd);

	void importMappings(const Dmix_pipelines_conf* newMappings);

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	~Dmix_pipelines_conf();

	torasu::Identifier getType() const override;
	Dmix_pipelines_conf* clone() const override;

	friend Rmix_pipelines;
};

class Rmix_pipelines : public torasu::tools::SimpleRenderable {
public:
	struct MixEntry {
		std::string pl;
		torasu::RenderableSlot rnd;
	};

private:
	torasu::tools::ManagedRenderableSlot defRnd;
	Dmix_pipelines_conf conf;

public:
	Rmix_pipelines(torasu::RenderableSlot def, std::initializer_list<MixEntry> mixes);
	~Rmix_pipelines();
	Identifier getType() override;

	RenderResult* render(RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_
