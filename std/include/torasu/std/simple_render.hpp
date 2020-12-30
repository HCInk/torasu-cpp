#ifndef STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_
#define STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

#include <torasu/render_tools.hpp>
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/LIcore_logger.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Dstring.hpp>

namespace torasu::tstd {

template<class T> struct SimpleResult {
	std::shared_ptr<RenderResult> rr;
	torasu::ResultStatus rStat;
	torasu::tools::CastedRenderSegmentResult<T> rs;
	torasu::ResultSegmentStatus segStat;
	T* result;

	bool check() {
		return result != nullptr &&
			   rStat == torasu::ResultStatus::ResultStatus_OK &&
			   segStat == torasu::ResultSegmentStatus::ResultSegmentStatus_OK;
	}

	std::string getInfo() {
		std::stringstream ss;
		ss << "RSTAT=" <<  std::to_string(rStat) << ", "
		   "SEGSTAT=" <<  std::to_string(segStat) << ", "
		   "RES=" << result;
		return ss.str();
	}
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

	torasu::tstd::LIcore_logger logger;
	LogInstruction li(&logger);

	RenderResult* rr = rib.runRender(tree, &rctx, ei.get(), li);

	// Finding results

	torasu::tools::CastedRenderSegmentResult<T> found = handle.getFrom(rr);
	return {std::shared_ptr<RenderResult>(rr), rr->getStatus(), found, found.getStatus(), found.getResult()};
}

template<class T> SimpleResult<T> simpleRenderChecked(Renderable* tree, std::string pl, torasu::ResultFormatSettings* format) {

	auto srr = simpleRender<T>(tree, pl, format);

	if (srr.result == nullptr)
		throw std::runtime_error(std::string() + "No result was generated! (" + srr.getInfo() + ")");

	if (!srr.check()) {
		std::cerr << "SimpleRender: The generated result may contain errors! (" << srr.getInfo() << ")" << std::endl;
	}

	return srr;
}

inline torasu::tstd::Dnum renderNum(Renderable* tree) {
	return *simpleRenderChecked<torasu::tstd::Dnum>(tree, TORASU_STD_PL_NUM, nullptr).result;
}


inline torasu::tstd::Dstring renderString(Renderable* tree) {
	return *simpleRenderChecked<torasu::tstd::Dstring>(tree, TORASU_STD_PL_STRING, nullptr).result;
}

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_
