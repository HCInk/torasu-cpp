#ifndef STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

namespace torasu::tstd {

class Rstring_file : public torasu::tools::SimpleRenderable {
private:
	torasu::tools::ManagedRenderableSlot srcRnd;

public:
	/**
	 * @brief  Provides a string as a file
	 * @param  src:	The string-source for the file
	 */
	explicit Rstring_file(torasu::RenderableSlot src);
	~Rstring_file();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_
