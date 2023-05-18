#ifndef STD_INCLUDE_TORASU_STD_RLIST_HPP_
#define STD_INCLUDE_TORASU_STD_RLIST_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/context_names.hpp>
#include <torasu/std/property_names.hpp>
#include <torasu/std/Dstring_pair.hpp>

namespace torasu::tstd {

class Rlist : public torasu::tools::SimpleRenderable {
private:
	/** @brief
	 * A: key in rctx to use for iteration
	 * B: pipeline on which the length should be provided on
	 */
	Dstring_pair data;
	std::map<size_t, tools::ManagedRenderableSlot> slots;
	size_t length = 0;

public:
	/**
	 * @brief  Create an Rlist
	 * @param  list: List elements
	 * @param  iteratorKey: key in rctx to use for iteration
	 * @param  lengthPipeline: pipeline on which the length should be provided on
	 */
	Rlist(std::initializer_list<RenderableSlot> list, std::string iteratorKey = TORASU_STD_CTX_IT, std::string lengthPipeline = TORASU_PROPERTY(TORASU_STD_PROP_IT_LENGTH));
	~Rlist();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RLIST_HPP_
