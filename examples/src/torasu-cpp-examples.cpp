// System
#include <iostream>
#include <map>
#include <string>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/json.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/render_tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/Rsubtract.hpp>
#include <torasu/std/Rjson_prop.hpp>
#include <torasu/std/Rnet_file.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/EIcore_runner.hpp>

#include "task-distribution-test.hpp"
#include "../boilerplate/execution-boilerplate.hpp"

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
	std::cout << "//" << std::endl
			  << "// DP Test" << std::endl
			  << "//" << std::endl;

	torasu::json dpJson  = {
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

void simpleRenderExample1() {

	//
	// Simple Render Example 1
	//

	cout << "//" << endl
		 << "// Simple Render Example 1" << endl
		 << "//" << endl;

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

	RenderContext rctx;

	RenderResult* rr = rib.runRender(&tree, &rctx, ei);

	// Finding results

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result.getResult()->getNum() << endl;

	// Cleaning

	delete rr;
	delete ei;

}

void simpleRenderExample2() {

	//
	// Simple Render Example 2
	//

	cout << "//" << endl
		 << "// Simple Render Example 2" << endl
		 << "//" << endl;

	// Creating "tree" to be rendered

	Rnum numA(0.1);
	Rnum numB(0.2);

	torasu::tstd::Rsubtract tree(&numA, &numB);

	// Creating the runner

	EIcore_runner runner;

	ExecutionInterface* ei = runner.createInterface();

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	// Running render based on instruction

	RenderContext rctx;

	RenderResult* rr = rib.runRender(&tree, &rctx, ei);

	// Finding results

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result.getResult()->getNum() << endl;

	// Cleaning

	delete rr;
	delete ei;

}

void jsonPropExample() {

	//
	// Json Prop Example
	//

	cout << "//" << endl
		 << "// Json Prop Example" << endl
		 << "//" << endl;
	

	torasu::tstd::Rnet_file jsonFile("https://dummy.restapiexample.com/api/v1/employee/1");
	torasu::tstd::Rjson_prop tree("data.employee_name", &jsonFile);

	// Creation of Runner / ExecutionInterface
	torasu::tstd::EIcore_runner runner;
	std::unique_ptr<torasu::ExecutionInterface> ei(runner.createInterface());

	// Creation of the instruction-builder (Defintion of segments)
	torasu::tools::RenderInstructionBuilder rib;
	auto resHandle = rib.addSegmentWithHandle<torasu::tstd::Dstring>(TORASU_STD_PL_STRING, nullptr);

	// Rendering / Fetching the Render-Result
	RenderContext rctx;
	std::unique_ptr<torasu::RenderResult> rr(rib.runRender(&tree, &rctx, ei.get()));
	auto rs = resHandle.getFrom(rr.get());

	// Evalulating the Result

	torasu::tstd::Dstring* strData = rs.getResult();
	if (rs.getResult() != nullptr) {
		std::cout << "EXEC-RESULT: \"" << strData->getString() << "\" - STATUS: " << rs.getStatus() << std::endl;
	} else {
		std::cout << "EXEC-RESULT: NONE/ERROR - STATUS: " << rs.getStatus() << std::endl;
	}
}

} // namespace torasu::texample

using namespace torasu::texample;

int main(int argc, char** argv) {

	checkLinkings();

	boilerplate_execution_initializer();

	simpleDpTest();

	simpleRenderExample1();

	simpleRenderExample2();

	jsonPropExample();

	// taskDistTest();

	return 0;
}
