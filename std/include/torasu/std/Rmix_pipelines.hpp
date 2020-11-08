#ifndef STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_
#define STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/DataPackable.hpp>

namespace torasu::tstd {

class Rmix_pipelines;

class Dmix_pipelines_conf : public torasu::DataPackable {

private:
	struct PipelineMapping {
		size_t id;
		std::string pl;
		torasu::Renderable* rnd; // Will not be exported/imported
	};
	
	// mappings (managed by applyMappings(...) and updateMapping(...))
	std::map<size_t, PipelineMapping*> mappingsById;
	std::map<std::string, PipelineMapping*> mappingsByPl;

	void applyMappings(std::vector<PipelineMapping> entries);
	void updateMapping(size_t id, torasu::Renderable* rnd);

	void importMappings(const Dmix_pipelines_conf* newMappings);

protected:
	void load() override;
	torasu::json makeJson() override;

public:
	~Dmix_pipelines_conf();

	std::string getIdent() override;

	friend Rmix_pipelines;
};

class Rmix_pipelines : public torasu::tools::SimpleRenderable {
public:
	struct MixEntry {
		std::string pl;
		Renderable* rnd;
	};

private:
	Renderable* defRnd;
	Dmix_pipelines_conf conf;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rmix_pipelines(Renderable* def, std::initializer_list<MixEntry> mixes);
	~Rmix_pipelines();

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

};
	
} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RMIX_PIPELINES_HPP_
