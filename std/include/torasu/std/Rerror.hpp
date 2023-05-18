#ifndef STD_INCLUDE_TORASU_STD_RERROR_HPP_
#define STD_INCLUDE_TORASU_STD_RERROR_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rstring.hpp>


namespace torasu::tstd {

class Rerror : public torasu::tools::SimpleRenderable {
private:
	torasu::tools::ManagedSlot<torasu::tstd::StringSlot> msgRnd;

public:
	explicit Rerror(torasu::tstd::StringSlot msg);
	~Rerror();
	torasu::Identifier getType() override;

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	const torasu::OptElementSlot setElement(std::string key, const torasu::ElementSlot* elem) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RERROR_HPP_
