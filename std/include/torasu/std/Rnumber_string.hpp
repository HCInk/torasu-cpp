#ifndef STD_INCLUDE_TORASU_STD_RNUMBER_STRING_HPP_
#define STD_INCLUDE_TORASU_STD_RNUMBER_STRING_HPP_

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
	torasu::tools::ManagedSlot<torasu::tstd::NumSlot> decimalsRnd;
	torasu::tools::ManagedSlot<torasu::tstd::NumSlot> minDigitsRnd;

public:
	/**
	 * @brief  Provides a number as a string
	 * @param  src:	The number-source for the string
	 * @param  decimals: Number of decimal-places
	 * @param  minDigits: Minimum number of digits (will be padded with zeros)
	 */
	explicit Rnumber_string(torasu::tstd::NumSlot src, torasu::tstd::NumSlot decimals = 0.0, torasu::tstd::NumSlot minDigits = 1.0);
	~Rnumber_string();
	Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RNUMBER_STRING_HPP_
