// 010-TestCase.cpp

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

// Testing framework            
#include <catch2/catch.hpp>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/RNum.hpp>
#include <torasu/std/DPNum.hpp>
#include <torasu/std/EICoreRunner.hpp>

using namespace torasu;
using namespace torasu::tstd;

TEST_CASE( "Simple numeric render test", "[single-file]" ) {

	// Creating "tree" to be rendered

	RNum rnum(1.1);

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<DPNum>("STD::PNUM", NULL);

	//	Create interface
	
	EICoreRunner runner;
	ExecutionInterface* ei = runner.createInterface();

	// Running render based on instruction

	RenderResult* rr = rib.runRender(&rnum, NULL, ei);

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