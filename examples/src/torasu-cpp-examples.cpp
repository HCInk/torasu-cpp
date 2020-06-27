// System
#include <iostream>
#include <map>
#include <string>

// Dev Dependencies
#include <nlohmann/json.hpp>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/render_tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/EIcore_runner.hpp>

#include "task-distribution-test.hpp"

using namespace std;
using namespace torasu;
using namespace torasu::tstd;

namespace torasu::texample {

void checkLinkings() {

	cout << "Checking core..." << endl;
	TORASU_check_core();
	cout << "Checking std..." << endl;
	TORASU_check_std();
	cout << "Everything seems good!" << endl;

}

void simpleDpTest() {

	nlohmann::json dpJson  = {
		{"ident", "torasu::testdp"},
		{"secondProp", "test"}
	};

	DPUniversal dpu(dpJson);

	std::cout << "ident: \"" << dpu.getIdent() << "\"" << std::endl;

	DataDump* dump = dpu.getData();

	if (dump->getFormat() == DDDataPointerType::DDDataPointerType_JSON_CSTR) {
		std::cout << "data:" << dump->getData().s << std::endl;
	} else {
		std::cerr << "unexpected DDDPT" << std::endl;
	}
	delete dump;

}

void simpleRenderExample() {

	//
	// Simple Render Example
	//

	// Creating "tree" to be rendered

	Rnum numA(0.1);
	Rnum numB(0.1);

	Rmultiply tree(&numA, &numB);

	// Creating the runner

	EIcore_runner runner;

	ExecutionInterface* ei = runner.createInterface();

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	// Running render based on instruction

	RenderResult* rr = rib.runRender(&tree, NULL, ei);

	// Finding results

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result.getResult()->getNum() << endl;

	// Cleaning

	delete rr;
	delete ei;

}

} // namespace torasu::texample

using namespace torasu::texample;

int main(int argc, char** argv) {

	checkLinkings();

	simpleDpTest();

	simpleRenderExample();

	// taskDistTest();

	return 0;
}
