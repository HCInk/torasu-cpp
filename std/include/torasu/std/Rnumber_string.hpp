#ifndef STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_

#include <string>
#include <map>

#include <torasu/torasu.hpp>
#include <torasu/slot_tools.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Rnum.hpp>

namespace torasu::tstd {

class Rnumber_string : public torasu::tools::SimpleRenderable {
private:
	torasu::tools::ManagedSlot<torasu::tstd::NumSlot> srcRnd;

public:
	/**
	 * @brief  Provides a number as a string
	 * @param  src:	The number-source for the string
	 */
	explicit Rnumber_string(torasu::tstd::NumSlot src);
	~Rnumber_string();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_FILE_HPP_
