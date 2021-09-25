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
#include <torasu/std/Rstring.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/Radd.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/Rsin.hpp>
#include <torasu/std/Rfallback.hpp>
#include <torasu/std/simple_render.hpp>

using namespace torasu;
using namespace torasu::tstd;

auto& IR = torasu::tools::inlineRenderable;

struct SimpleNumeric {

	void simpleNumeric() {
		// Creating "tree" to be rendered

		Rnum rnum(1.1);

		// Creating instruction

		torasu::ResultSettings rs("STD::PNUM", NULL);

		//	Create interface
		
		LIcore_logger logger;
		LogInstruction li(&logger);
		EIcore_runner runner;
		ExecutionInterface* ei = runner.createInterface();

		// Running render based on instruction

		RenderContext rctx;

		RenderResult* rr = ei->fetchRenderResult(ei->enqueueRender(&rnum, &rctx, &rs, li, 0));

		// Finding results

		CHECK( rr->getStatus() == RenderResultStatus::RenderResultStatus_OK );
		
		if (rr->getStatus() == RenderResultStatus::RenderResultStatus_OK) {
				
			tools::CastedRenderSegmentResult<tstd::Dnum> result(rr);

			CHECK( result.getStatus() == RenderResultStatus::RenderResultStatus_OK );
			
			if (result.getStatus() == RenderResultStatus::RenderResultStatus_OK) {
				CHECK( result.getResult()->getNum() == 1.1 );
			}
			
		}

		delete rr;

		delete ei;

	}

	void advancedNumeric() {

		// Creating "tree" to be rendered

		Rnum rnum(1.1);
		Radd add(&rnum, 10);
		Rmultiply mul(&add, IR( new Rsin(90) ));

		auto& tree = mul;

		// Creating instruction

		torasu::ResultSettings rs("STD::PNUM", NULL);

		//	Create interface
		
		LIcore_logger logger;
		LogInstruction li(&logger);
		EIcore_runner runner;
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());

		// Running render based on instruction

		RenderContext rctx;

		std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(ei->enqueueRender(&tree, &rctx, &rs, li, 0)));

		// Finding results

		CHECK( rr->getStatus() == RenderResultStatus::RenderResultStatus_OK );
		
		if (rr->getStatus() == RenderResultStatus::RenderResultStatus_OK) {
				
			tools::CastedRenderSegmentResult<tstd::Dnum> result(rr.get());

			CHECK( result.getStatus() == RenderResultStatus::RenderResultStatus_OK );
			
			if (result.getStatus() == RenderResultStatus::RenderResultStatus_OK) {
				CHECK( result.getResult()->getNum() == 11.1 );
			}
			
		}
	}
};

METHOD_AS_TEST_CASE( SimpleNumeric::simpleNumeric , "Simple numeric render test", "[class]" )
METHOD_AS_TEST_CASE( SimpleNumeric::advancedNumeric , "Advanced numeric render test", "[class]" )


void numericBurstTest(ExecutionInterface* ei, size_t bursts, size_t perBurst) {

	LIcore_logger logger;
	LogInstruction li(&logger);

	// Creating "tree" to be rendered

	Rnum rnum(1.1);
	Radd add(&rnum, 10);
	Rmultiply mul(&add, IR( new Rsin(90) ));

	auto& tree = mul;

	// Creating instruction


	torasu::ResultSettings rs("STD::PNUM", NULL);

	// Running render based on instruction

	std::vector<size_t> rids(perBurst);

	RenderContext rctx;
	for (size_t i = 0; i < bursts; i++) {

		for (size_t rn = 0; rn < perBurst; rn++) {
			rids[rn] = ei->enqueueRender(&tree, &rctx, &rs, li, 0);
		}
		

		for (size_t rn = 0; rn < perBurst; rn++) {
			std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rids[rn]));


			// Finding results

			CHECK( rr->getStatus() == RenderResultStatus::RenderResultStatus_OK );
			
			if (rr->getStatus() == RenderResultStatus::RenderResultStatus_OK) {
					
				tools::CastedRenderSegmentResult<tstd::Dnum> result(rr.get());

				CHECK( result.getStatus() == RenderResultStatus::RenderResultStatus_OK );
				
				if (result.getStatus() == RenderResultStatus::RenderResultStatus_OK) {
					CHECK( result.getResult()->getNum() == 11.1 );
				}
				
			}
		}
	}

}

void numericBurstTestAsync(ExecutionInterface* ei, size_t bursts, size_t perBurst, size_t poolSize) {

	LIcore_logger logger;
	LogInstruction li(&logger);

	// Creating "tree" to be rendered

	Rnum rnum(1.1);
	Radd add(&rnum, 10);
	Rmultiply mul(&add, IR( new Rsin(90) ));

	auto& tree = mul;

	// Creating instruction

	torasu::ResultSettings rs("STD::PNUM", NULL);

	// Running render based on instruction

	auto func = [&tree, ei, li, &rs, bursts, perBurst](){

		std::vector<size_t> rids(perBurst);

		RenderContext rctx;
		for (size_t i = 0; i < bursts; i++) {

			for (size_t rn = 0; rn < perBurst; rn++) {
				rids[rn] = ei->enqueueRender(&tree, &rctx, &rs, li, 0);
			}
			

			for (size_t rn = 0; rn < perBurst; rn++) {
				std::unique_ptr<RenderResult> rr(ei->fetchRenderResult(rids[rn]));

				// Finding results

				CHECK( rr->getStatus() == torasu::RenderResultStatus_OK );
				
				if (rr->getStatus() == torasu::RenderResultStatus_OK) {
						
					tools::CastedRenderSegmentResult<tstd::Dnum> result(rr.get());

					CHECK( result.getStatus() == RenderResultStatus::RenderResultStatus_OK );
					
					if (result.getStatus() == RenderResultStatus::RenderResultStatus_OK) {
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

struct RunnerTest {
	// 2K Numeric Burst (0-thread)
	void b_2K_0t() {
		EIcore_runner runner;
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 2K Numeric burst (noQueue)
	void b_2K_nq() {
		EIcore_runner runner(false);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 2K Numeric burst (noQueue, not concurrent)
	void b_2K_nqnc() {
		EIcore_runner runner(false, false);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 2K Numeric burst (1-thread)
	void b_2K_1t() {
		EIcore_runner runner((size_t)1);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 2K Numeric burst (2-thread)
	void b_2K_2t() {
		EIcore_runner runner((size_t)2);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 2K Numeric burst (8-thread)
	void b_2K_8t() {
		EIcore_runner runner((size_t)8);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 2*1000, 1);
	}

	// 500*100 Numeric Burst (0-thread)
	void b_500b100_0t() {
		EIcore_runner runner;
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 500, 100);
	}

	// 500*100 Numeric Burst (1-thread)
	void b_500b100_1t() {
		EIcore_runner runner((size_t)1);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 500, 100);
	}

	// 500*100 Numeric Burst (2-thread)
	void b_500b100_2t() {
		EIcore_runner runner((size_t)2);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 500, 100);
	}

	// 500*100 Numeric Burst (8-thread)
	void b_500b100_8t() {
		EIcore_runner runner((size_t)8);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTest(ei.get(), 500, 100);
	}


	// 500*100*1 Numeric Burst (0-thread)
	void b_500b100t1_0t() {
		EIcore_runner runner((size_t)0);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 500, 100, 1);
	}


	// 500*10*10 Numeric Burst (0-thread)
	void b_500b10t10_0t() {
		EIcore_runner runner((size_t)0);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 500, 10, 10);
	}

	// 500*10*10 Numeric Burst (1-thread)
	void b_500b10t10_1t() {
		EIcore_runner runner((size_t)1);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 500, 10, 10);
	}

	// 500*10*10 Numeric Burst (2-thread)
	void b_500b10t10_2t() {
		EIcore_runner runner((size_t)2);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 500, 10, 10);
	}

	// 500*10*10 Numeric Burst (8-thread)
	void b_500b10t10_8t() {
		EIcore_runner runner((size_t)8);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 500, 10, 10);
	}


	// 50*10*100 Numeric Burst (0-thread)
	void b_50b10t100_0t() {
		EIcore_runner runner((size_t)0);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 10, 100);
	}

	// 50*10*100 Numeric Burst (1-thread)
	void b_50b10t100_1t() {
		EIcore_runner runner((size_t)1);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 10, 100);
	}

	// 50*10*100 Numeric Burst (2-thread)
	void b_50b10t100_2t() {
		EIcore_runner runner((size_t)2);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 10, 100);
	}

	// 50*10*100 Numeric Burst (8-thread)
	void b_50b10t100_8t() {
		EIcore_runner runner((size_t)8);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 10, 100);
	}

	// 50*100*100 Numeric Burst (0-thread)
	void b_50b100t100_0t() {
		EIcore_runner runner((size_t)0);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 100, 100);
	}

	// 50*100*100 Numeric Burst (1-thread)
	void b_50b100t100_1t() {
		EIcore_runner runner((size_t)1);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 100, 100);
	}

	// 50*100*100 Numeric Burst (2-thread)
	void b_50b100t100_2t() {
		EIcore_runner runner((size_t)2);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 100, 100);
	}

	// 50*100*100 Numeric Burst (8-thread)
	void b_50b100t100_8t() {
		EIcore_runner runner((size_t)8);
		std::unique_ptr<ExecutionInterface> ei(runner.createInterface());
		numericBurstTestAsync(ei.get(), 50, 100, 100);
	}

};

METHOD_AS_TEST_CASE ( RunnerTest::b_2K_0t, "2K Numeric Burst (0-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_2K_nq, "2K Numeric burst (noQueue)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_2K_nqnc, "2K Numeric burst (noQueue, not concurrent)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_2K_1t, "2K Numeric burst (1-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_2K_2t, "2K Numeric burst (2-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_2K_8t, "2K Numeric burst (8-thread)", "[class]" )

METHOD_AS_TEST_CASE ( RunnerTest::b_500b100_0t, "500*100 Numeric Burst (0-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b100_1t, "500*100 Numeric Burst (1-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b100_2t, "500*100 Numeric Burst (2-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b100_8t, "500*100 Numeric Burst (8-thread)", "[class]" )

METHOD_AS_TEST_CASE ( RunnerTest::b_500b100t1_0t, "500*100*1 Numeric Burst (0-thread)", "[class]" )

METHOD_AS_TEST_CASE ( RunnerTest::b_500b10t10_0t, "500*10*10 Numeric Burst (0-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b10t10_1t, "500*10*10 Numeric Burst (1-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b10t10_2t, "500*10*10 Numeric Burst (2-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_500b10t10_8t, "500*10*10 Numeric Burst (8-thread)", "[class]" )

METHOD_AS_TEST_CASE ( RunnerTest::b_50b10t100_0t, "50*10*100 Numeric Burst (0-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b10t100_1t, "50*10*100 Numeric Burst (1-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b10t100_2t, "50*10*100 Numeric Burst (2-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b10t100_8t, "50*10*100 Numeric Burst (8-thread)", "[class]" )

METHOD_AS_TEST_CASE ( RunnerTest::b_50b100t100_0t, "50*100*100 Numeric Burst (0-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b100t100_1t, "50*100*100 Numeric Burst (1-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b100t100_2t, "50*100*100 Numeric Burst (2-thread)", "[class]" )
METHOD_AS_TEST_CASE ( RunnerTest::b_50b100t100_8t, "50*100*100 Numeric Burst (8-thread)", "[class]" )

class FallbackTest {
public:
	static void singleFallback() {
		torasu::tstd::Rfallback fallback(
			{
				IR(new torasu::tstd::Rnum(10))
			}
		);

		auto srr = torasu::tstd::simpleRender<torasu::tstd::Dnum>(&fallback, TORASU_STD_PL_NUM, nullptr);

		CHECK(srr.rStat == RenderResultStatus::RenderResultStatus_OK);
		CHECK(srr.result != nullptr);
		CHECK(srr.result->getNum() == 10);

	}


	static void firstFallback() {
		torasu::tstd::Rfallback fallback(
			{
				IR(new torasu::tstd::Rnum(3)),
				IR(new torasu::tstd::Rnum(5)),
			}
		);

		auto srr = torasu::tstd::simpleRender<torasu::tstd::Dnum>(&fallback, TORASU_STD_PL_NUM, nullptr);

		CHECK(srr.rStat == RenderResultStatus::RenderResultStatus_OK);
		CHECK(srr.result != nullptr);
		CHECK(srr.result->getNum() == 3);

	}

	static void invalidSegmentFallback() {
		torasu::tstd::Rfallback fallback(
			{
				IR( new torasu::tstd::Rstring("test") ),
				IR( new torasu::tstd::Rnum(10) )
			}
		);

		auto srr = torasu::tstd::simpleRender<torasu::tstd::Dnum>(&fallback, TORASU_STD_PL_NUM, nullptr);

		CHECK(srr.rStat == RenderResultStatus::RenderResultStatus_OK);
		CHECK(srr.result != nullptr);
		CHECK(srr.result->getNum() == 10);

	}

};

METHOD_AS_TEST_CASE ( FallbackTest::singleFallback, "Single Fallback", "[class]" )
METHOD_AS_TEST_CASE ( FallbackTest::firstFallback, "First fallback", "[class]" )
METHOD_AS_TEST_CASE ( FallbackTest::invalidSegmentFallback, "Invalid segment fallback", "[class]" )
