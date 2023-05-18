#ifndef STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Rstring.hpp>

namespace torasu::tstd {

class Rnet_file : public tools::SimpleRenderable {
private:
	tools::ManagedSlot<StringSlot> urlRnd;
	tools::ManagedSlot<StringSlot> headersRnd;

public:
	explicit Rnet_file(StringSlot url, StringSlot headers = StringSlot());
	~Rnet_file();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;

};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNET_FILE_HPP_
