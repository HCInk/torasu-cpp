#ifndef STD_INCLUDE_TORASU_STD_RSTRING_CONCAT_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_CONCAT_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring.hpp>
#include <torasu/std/Rstring.hpp>

namespace torasu::tstd {

class Rstring_concat : public torasu::tools::SimpleRenderable {
public:
	/** @brief Key of rctx-value, which contains the value of the list */
	static const char* RCTX_KEY_VALUE;
private:
	torasu::tools::ManagedRenderableSlot listRnd;
	torasu::tools::ManagedRenderableSlot genRnd;

public:
	/**
	 * @brief  Concats contents of the provided list
	 * @param  list: The list-provider.
	 * @param  gen: The generator, which gets rendered for every list item.
	 * 					It's result will be concatted into the result string
	 */
	explicit Rstring_concat(torasu::RenderableSlot list, torasu::RenderableSlot gen);
	~Rstring_concat();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_CONCAT_HPP_
