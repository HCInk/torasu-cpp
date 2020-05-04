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
#include <torasu/std/RNum.hpp>
#include <torasu/std/DPNum.hpp>


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

	RNum rnum(1.1);

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<DPNum>("STD::PNUM", NULL);

	// Running render based on instruction

	RenderInstruction  ri = rib.getInstruction(NULL);

	RenderResult* rr = rnum.render(&ri);

	// Finding results

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result->getResult()->getNum() << endl;

	// Cleaning

	delete result;
	delete rr;

}

} // namespace torasuexamples

using namespace torasu::texample;

int main(int argc, char **argv) {

	checkLinkings();

	simpleDpTest();

	simpleRenderExample();

	return 0;
}
