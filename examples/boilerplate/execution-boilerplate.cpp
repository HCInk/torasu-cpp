#include <iostream>
#include <memory>

#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

#include <torasu/std/pipeline_names.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/EIcore_runner.hpp>

#include "Dboilerplate.hpp"
#include "Rboilerplate.hpp"

namespace torasu::texample {

void boilerplate_execution_async(torasu::Renderable* tree, torasu::ExecutionInterface* ei, torasu::RenderContext* rctx) {

	// Creation of the instruction-builder (Defintion of segments)

	torasu::tools::RenderInstructionBuilder rib;
	auto resHandle = rib.addSegmentWithHandle<torasu::tstd::Dnum>(TORASU_STD_PL_NUM, nullptr /*TODO format here*/);

	// Enqueueing the render with generated instruction
	auto rndId = rib.enqueueRender(tree, rctx, ei);

	// Fetching the Render-Result
	std::unique_ptr<torasu::RenderResult> rr(ei->fetchRenderResult(rndId));
	auto rs = resHandle.getFrom(rr.get());

	// Evalulating the Result

	torasu::tstd::Dnum* numData = rs.getResult();
	if (rs.getResult() != nullptr) {
		double num = numData->getNum();
		std::cout << "EXEC-RESULT: " << num << " - STATUS: " << rs.getStatus() << std::endl;
	} else {
		std::cout << "EXEC-RESULT: NONE/ERROR - STATUS: " << rs.getStatus() << std::endl;
	}

}

void boilerplate_execution_sync(torasu::Renderable* tree, torasu::ExecutionInterface* ei, torasu::RenderContext* rctx) {

	// Creation of the instruction-builder (Defintion of segments)

	torasu::tools::RenderInstructionBuilder rib;
	auto resHandle = rib.addSegmentWithHandle<torasu::tstd::Dnum>(TORASU_STD_PL_NUM, nullptr /*TODO format here*/);

	// Rendering / Fetching the Render-Result
	std::unique_ptr<torasu::RenderResult> rr(rib.runRender(tree, rctx, ei));
	auto rs = resHandle.getFrom(rr.get());

	// Evalulating the Result

	torasu::tstd::Dnum* numData = rs.getResult();
	if (rs.getResult() != nullptr) {
		double num = numData->getNum();
		std::cout << "EXEC-RESULT: " << num << " - STATUS: " << rs.getStatus() << std::endl;
	} else {
		std::cout << "EXEC-RESULT: NONE/ERROR - STATUS: " << rs.getStatus() << std::endl;
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

	// Run the actual render
	RenderContext rctx;
	boilerplate_execution_async(&tree, ei.get(), &rctx);

}

} // namespace torasu::texample