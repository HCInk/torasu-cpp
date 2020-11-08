#ifndef STD_INCLUDE_TORASU_STD_RRCTX_VALUE_HPP_
#define STD_INCLUDE_TORASU_STD_RRCTX_VALUE_HPP_

#include <torasu/torasu.hpp>
#include <torasu/SimpleRenderable.hpp>

#include <torasu/std/Dstring_pair.hpp>

namespace torasu::tstd {

class Rrctx_value : public torasu::tools::SimpleRenderable {
private:
	Dstring_pair mapping; // A: valueKey B: pipelineName

protected:
	torasu::ResultSegment* renderSegment(torasu::ResultSegmentSettings* resSettings, torasu::RenderInstruction* ri) override;

public:
	Rrctx_value(std::string valueKey, std::string pipelineName);
	~Rrctx_value();

	torasu::DataResource* getData() override;
	void setData(torasu::DataResource* data) override;
};

} // namespace torasu::tstd

#endif // STD_INCLUDE_TORASU_STD_RRCTX_VALUE_HPP_
