#include <iostream>
#include <memory>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/LIcore_logger.hpp>

#include "Dboilerplate.hpp"
#include "Rboilerplate.hpp"

namespace torasu::texample {

void boilerplate_execution_async(torasu::Renderable* tree, torasu::ExecutionInterface* ei, torasu::RenderContext* rctx, LogInstruction li) {
	tools::LogInfoRefBuilder lrib(li);

	// Enqueueing the render with generated instruction
	torasu::ResultSettings rs(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
	auto rndId = ei->enqueueRender(tree, rctx, &rs, li, 0);

	// Fetching the Render-Result
	std::unique_ptr<torasu::RenderResult> rr(ei->fetchRenderResult(rndId));
	tools::CastedRenderSegmentResult<torasu::tstd::Dnum> result(rr.get(), &lrib);

	// Evalulating the Result

	torasu::tstd::Dnum* numData = result.getResult();
	if (result.getResult() != nullptr) {
		double num = numData->getNum();
		std::cout << "EXEC-RESULT: " << num << " - STATUS: " << result.getStatus() << std::endl;
	} else {
		std::cout << "EXEC-RESULT: NONE/ERROR - STATUS: " << result.getStatus() << std::endl;
	}

}

void boilerplate_execution_sync(torasu::Renderable* tree, torasu::ExecutionInterface* ei, torasu::RenderContext* rctx, LogInstruction li) {
	tools::LogInfoRefBuilder lrib(li);

	// Rendering / Fetching the Render-Result
	torasu::ResultSettings rs(TORASU_STD_PL_NUM, torasu::tools::NO_FORMAT);
	std::unique_ptr<torasu::RenderResult> rr(ei->fetchRenderResult(ei->enqueueRender(tree, rctx, &rs, li, 0)));
	tools::CastedRenderSegmentResult<torasu::tstd::Dnum> result(rr.get(), &lrib);

	// Evalulating the Result

	torasu::tstd::Dnum* numData = result.getResult();
	if (result.getResult() != nullptr) {
		double num = numData->getNum();
		std::cout << "EXEC-RESULT: " << num << " - STATUS: " << result.getStatus() << std::endl;
	} else {
		std::cout << "EXEC-RESULT: NONE/ERROR - STATUS: " << result.getStatus() << std::endl;
	}

}

void boilerplate_execution_initializer() {

	std::cout << "//" << std::endl
			  << "// Boilerplate Execution" << std::endl
			  << "//" << std::endl;

	// Creation of tree

	auto* one = new Dboilerplate("TEST", 2); // Data Payload for renderable (will be managed by Renderable)
	torasu::tstd::Rnum two(-3); // Sub-Renderable to rendered from Rboilerplate

	Rboilerplate tree(one, &two); // Root for the tree

	// Creation of Runner / ExecutionInterface
	torasu::tstd::EIcore_runner runner;
	std::unique_ptr<torasu::ExecutionInterface> ei(runner.createInterface());
	torasu::tstd::LIcore_logger logger;
	LogInstruction li(&logger);

	// Run the actual render
	RenderContext rctx;
	boilerplate_execution_async(&tree, ei.get(), &rctx, li);

}

} // namespace torasu::texample