#ifndef STD_INCLUDE_TORASU_STD_RSTRING_HPP_
#define STD_INCLUDE_TORASU_STD_RSTRING_HPP_

#include <string>

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/spoilsD.hpp>

namespace torasu::tstd {

class Rstring : public torasu::tools::SimpleRenderable {
private:
	Dstring* str;

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	explicit Rstring(std::string str);
	~Rstring();

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RSTRING_HPP_
