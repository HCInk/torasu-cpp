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
#include <torasu/std/Radd.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/Rsin.hpp>

using namespace torasu;
using namespace torasu::tstd;

TEST_CASE( "Simple numeric render test", "[simple-numeric]" ) {

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

auto& IR = torasu::tools::inlineRenderable;

TEST_CASE( "Advanced numeric render test", "[simple-numeric]" ) {

	// Creating "tree" to be rendered

	Rnum rnum(1.1);
	Radd add(&rnum, 10);
	Rmultiply mul(&add, IR( new Rsin(90) ));

	auto& tree = mul;

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	//	Create interface
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	// Running render based on instruction

	RenderContext rctx;

	std::unique_ptr<RenderResult> rr(rib.runRender(&tree, &rctx, ei.get()));

	// Finding results

	CHECK( rr->getStatus() == ResultStatus::ResultStatus_OK );
	
	if (rr->getStatus() == ResultStatus::ResultStatus_OK) {
			
		auto result = handle.getFrom(rr.get());

		CHECK( result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK );
		
		if (result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK) {
			CHECK( result.getResult()->getNum() == 11.1 );
		}
		
	}

}

void numericBurstTest(ExecutionInterface* ei, size_t bursts, size_t perBurst) {

	// Creating "tree" to be rendered

	Rnum rnum(1.1);
	Radd add(&rnum, 10);
	Rmultiply mul(&add, IR( new Rsin(90) ));

	auto& tree = mul;

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	// Running render based on instruction

	std::vector<size_t> rids(perBurst);

	RenderContext rctx;
	for (size_t i = 0; i < bursts; i++) {

		for (size_t rn = 0; rn < perBurst; rn++) {
			rids[rn] = rib.enqueueRender(&tree, &rctx, ei);
		}
		

		for (size_t rn = 0; rn < perBurst; rn++) {
			std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rids[rn]));

			// Finding results

			CHECK( rr->getStatus() == ResultStatus::ResultStatus_OK );
			
			if (rr->getStatus() == ResultStatus::ResultStatus_OK) {
					
				auto result = handle.getFrom(rr.get());

				CHECK( result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK );
				
				if (result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK) {
					CHECK( result.getResult()->getNum() == 11.1 );
				}
				
			}
		}
	}

}

void numericBurstTestAsync(ExecutionInterface* ei, size_t bursts, size_t perBurst, size_t poolSize) {

	// Creating "tree" to be rendered

	Rnum rnum(1.1);
	Radd add(&rnum, 10);
	Rmultiply mul(&add, IR( new Rsin(90) ));

	auto& tree = mul;

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	// Running render based on instruction

	auto func = [&tree, ei, &rib, &handle, bursts, perBurst](){

		std::vector<size_t> rids(perBurst);

		RenderContext rctx;
		for (size_t i = 0; i < bursts; i++) {

			for (size_t rn = 0; rn < perBurst; rn++) {
				rids[rn] = rib.enqueueRender(&tree, &rctx, ei);
			}
			

			for (size_t rn = 0; rn < perBurst; rn++) {
				std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rids[rn]));

				// Finding results

				CHECK( rr->getStatus() == ResultStatus::ResultStatus_OK );
				
				if (rr->getStatus() == ResultStatus::ResultStatus_OK) {
						
					auto result = handle.getFrom(rr.get());

					CHECK( result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK );
					
					if (result.getStatus() == ResultSegmentStatus::ResultSegmentStatus_OK) {
						CHECK( result.getResult()->getNum() == 11.1 );
					}
					
				}
			}
		}
	};

	std::vector<std::thread> pool;

	pool.push_back(std::thread(func));

	for (auto& thread : pool) {
		thread.join();
	}


}

TEST_CASE( "2K Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}

TEST_CASE( "2K Numeric burst (noQueue)", "[runner-test]" ) {
	
	EIcore_runner runner(false);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}


TEST_CASE( "2K Numeric burst (noQueue, not concurrent)", "[runner-test]" ) {
	
	EIcore_runner runner(false, false);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}

TEST_CASE( "2K Numeric burst (1-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)1);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}

TEST_CASE( "2K Numeric burst (2-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)2);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}

TEST_CASE( "2K Numeric burst (8-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)8);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 2*1000, 1);

}

TEST_CASE( "500*100 Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 500, 100);

}

TEST_CASE( "500*100 Numeric Burst (1-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)1);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 500, 100);

}

TEST_CASE( "500*100 Numeric Burst (2-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)2);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 500, 100);

}

TEST_CASE( "500*100 Numeric Burst (8-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)8);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTest(ei.get(), 500, 100);

}

TEST_CASE( "500*100*1 Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 500, 100, 1);

}

TEST_CASE( "500*10*10 Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 500, 10, 10);

}

TEST_CASE( "500*10*10 Numeric Burst (1-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)1);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 500, 10, 10);

}

TEST_CASE( "500*10*10 Numeric Burst (2-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)2);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 500, 10, 10);

}

TEST_CASE( "500*10*10 Numeric Burst (8-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)8);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 500, 10, 10);

}

TEST_CASE( "50*10*100 Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 10, 100);

}

TEST_CASE( "50*10*100 Numeric Burst (1-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)1);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 10, 100);

}

TEST_CASE( "50*10*100 Numeric Burst (2-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)2);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 10, 100);

}

TEST_CASE( "50*10*100 Numeric Burst (8-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)8);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 10, 100);

}

TEST_CASE( "50*100*100 Numeric Burst (0-thread)", "[runner-test]" ) {
	
	EIcore_runner runner;
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 100, 100);

}

TEST_CASE( "50*100*100 Numeric Burst (1-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)1);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 100, 100);

}

TEST_CASE( "50*100*100 Numeric Burst (2-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)2);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 100, 100);

}

TEST_CASE( "50*100*100 Numeric Burst (8-thread)", "[runner-test]" ) {
	
	EIcore_runner runner((size_t)8);
	std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

	numericBurstTestAsync(ei.get(), 50, 100, 100);

}