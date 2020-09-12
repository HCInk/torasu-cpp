// 010-TestCase.cpp

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

// Testing framework            
#include <catch2/catch.hpp>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/render_tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/EIcore_runner.hpp>

using namespace torasu;
using namespace torasu::tstd;

TEST_CASE( "Simple numeric render test", "[single-file]" ) {

	// Creating "tree" to be rendered

	Rnum rnum(1.1);

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	//	Create interface
	
	EIcore_runner runner;
	ExecutionInterface* ei = runner.createInterface();

	// Running render based on instruction

	RenderContext rctx;

	RenderResult* rr = rib.runRender(&rnum, &rctx, ei);

	// Finding results

	CHECK( rr->getStatus() == ResultStatus::ResultStatus_OK );
	
	if (rr->getStatus() == ResultStatus::ResultStatus_OK) {
			
		auto result = handle.getFrom(rr);

		CHECK( result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK );
		
		if (result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK) {
			CHECK( result.getResult()->getNum() == 1.1 );
		}
		
	}

	delete rr;

	delete ei;

}