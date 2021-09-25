#ifndef STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_

#include <string>
#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class Rlocal_file : public tools::SimpleRenderable {
private:
	std::string path;

public:
	explicit Rlocal_file(std::string path);
	virtual ~Rlocal_file();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RLOCAL_FILE_HPP_
