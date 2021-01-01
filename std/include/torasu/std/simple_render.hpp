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

template<class T> SimpleResult<T> simpleRender(Renderable* tree, std::string pl, torasu::ResultFormatSettings* format, LogInstruction* li = nullptr) {

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<T>(pl, format);

	// Create interface

	EIcore_runner runner;
	std::unique_ptr<torasu::ExecutionInterface> ei(runner.createInterface());

	// Running render based on instruction

	RenderContext rctx;

	std::unique_ptr<torasu::tstd::LIcore_logger> logger;
	LogInstruction logInstr(nullptr);
	if (li == nullptr) {
		logger = std::unique_ptr<torasu::tstd::LIcore_logger>(new torasu::tstd::LIcore_logger());
		logInstr.logger = logger.get();
	} else {
		logInstr = LogInstruction(*li);
	}

	RenderResult* rr = rib.runRender(tree, &rctx, ei.get(), logInstr);

	// Finding results

	torasu::tools::CastedRenderSegmentResult<T> found = handle.getFrom(rr);
	return {std::shared_ptr<RenderResult>(rr), rr->getStatus(), found, found.getStatus(), found.getResult()};
}

template<class T> SimpleResult<T> simpleRenderChecked(Renderable* tree, std::string pl, torasu::ResultFormatSettings* format, LogInstruction* li = nullptr) {

	auto srr = simpleRender<T>(tree, pl, format, li);

	if (srr.result == nullptr)
		throw std::runtime_error(std::string() + "No result was generated! (" + srr.getInfo() + ")");

	if (!srr.check()) {
		std::cerr << "SimpleRender: The generated result may contain errors! (" << srr.getInfo() << ")" << std::endl;
	}

	return srr;
}

inline torasu::tstd::Dnum renderNum(Renderable* tree, LogInstruction* li = nullptr) {
	return *simpleRenderChecked<torasu::tstd::Dnum>(tree, TORASU_STD_PL_NUM, nullptr, li).result;
}


inline torasu::tstd::Dstring renderString(Renderable* tree, LogInstruction* li = nullptr) {
	return *simpleRenderChecked<torasu::tstd::Dstring>(tree, TORASU_STD_PL_STRING, nullptr, li).result;
}

} // namespace torasu::tstd


#endif // STD_INCLUDE_TORASU_STD_SIMPLE_RENDER_HPP_
