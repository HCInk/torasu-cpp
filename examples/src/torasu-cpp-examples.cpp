// System
#include <iostream>
#include <map>
#include <string>

// Dev Dependencies
#include <nlohmann/json.hpp>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/DPNum.hpp>
#include <torasu/std/RNum.hpp>
#include <torasu/std/RMultiply.hpp>
#include <torasu/std/EICoreRunner.hpp>


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

	DataDump dump = dpu.getData();

	if (dump.getFormat() == DDDataPointerType::DDDataPointerType_JSON_CSTR) {
		std::cout << "data:" << dump.getData().s << std::endl;
	} else {
		std::cerr << "unexpected DDDPT" << std::endl;
	}

}

void simpleRenderExample() {

	//
	// Simple Render Example
	//

	// Creating "tree" to be rendered

	RNum numA(0.1);
	RNum numB(0.1);

	RMultiply tree(&numA, &numB);

	// Creating the runner

	EICoreRunner runner;

	ExecutionInterface* ei = runner.createInterface();

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<DPNum>("STD::PNUM", NULL);

	// Running render based on instruction

	RenderResult* rr = rib.runRender(&tree, NULL, ei);

	// Finding results

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result->getResult()->getNum() << endl;

	// Cleaning

	delete result;
	delete rr;
	delete ei;

}

} // namespace torasu::texample

using namespace torasu::texample;

int main(int argc, char **argv) {

	checkLinkings();

	simpleDpTest();

	simpleRenderExample();

	return 0;
}
