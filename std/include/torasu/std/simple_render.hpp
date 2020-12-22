#ifndef STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_
#define STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_

#include <memory>
#include <string>

#include <torasu/render_tools.hpp>
#include <torasu/std/EIcore_runner.hpp>

namespace torasu::tstd {

template<class T> class SimpleResult {
public:
	std::shared_ptr<RenderResult> rr;
	torasu::ResultStatus rStat;
	torasu::tools::CastedRenderSegmentResult<T> rs;
	torasu::ResultSegmentStatus segStat;
	T* result;
};

template<class T> SimpleResult<T> simpleRender(Renderable* tree, std::string pl, torasu::ResultFormatSettings* format) {

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<T>(pl, format);

	// Create interface

	EIcore_runner runner;
	std::unique_ptr<torasu::ExecutionInterface> ei(runner.createInterface());

	// Running render based on instruction

	RenderContext rctx;

	RenderResult* rr = rib.runRender(tree, &rctx, ei.get());

	// Finding results

	torasu::tools::CastedRenderSegmentResult<T> found = handle.getFrom(rr);
	return SimpleResult<T>(std::shared_ptr<RenderResult>(rr), rr->getStatus(), found, found.getStatus(), found.getResult());
}

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_
