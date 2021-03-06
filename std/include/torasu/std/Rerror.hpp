#ifndef STD_INCLUDE_TORASU_STD_RERROR_HPP_
#define STD_INCLUDE_TORASU_STD_RERROR_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>
#include <torasu/slot_tools.hpp>

#include <torasu/std/Rstring.hpp>


namespace torasu::tstd {

class Rerror : public torasu::tools::NamedIdentElement,
	public torasu::tools::SimpleDataElement,
	public torasu::tools::ReadylessElement,
	public torasu::Renderable {
private:
	torasu::tools::ManagedSlot<torasu::tstd::StringSlot> msgRnd;

public:
	explicit Rerror(torasu::tstd::StringSlot msg);
	~Rerror();

	torasu::RenderResult* render(torasu::RenderInstruction* ri) override;

	torasu::ElementMap getElements() override;
	void setElement(std::string key, Element* elem) override;

};

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_RERROR_HPP_
