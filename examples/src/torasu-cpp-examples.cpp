// System
#include <iostream>
#include <map>
#include <string>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/json.hpp>
#include <torasu/DataPackable.hpp>
#include <torasu/render_tools.hpp>
#include <torasu/slot_tools.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/Dnum.hpp>
#include <torasu/std/Rnum.hpp>
#include <torasu/std/Rmultiply.hpp>
#include <torasu/std/Rsubtract.hpp>
#include <torasu/std/Rstring.hpp>
#include <torasu/std/Rjson_prop.hpp>
#include <torasu/std/Rnet_file.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/Rsin.hpp>

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

	DataDump* dump = dpu.dumpResource();

	if (dump->isJson()) {
		std::cout << "data:" << dump->getData().s << std::endl;
	} else {
		std::cerr << "Data is not JSON!" << std::endl;
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

	torasu::tstd::Rstring url("https://dummy.restapiexample.com/api/v1/employee/1");
	torasu::tstd::Rnet_file jsonFile(&url);
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

void mathExample() {

	cout << "//" << endl
		 << "// Math Example" << endl
		 << "//" << endl;

	Rnum num(10);
	Rsin sin(&num);

	auto& tree = sin;

	torasu::tstd::EIcore_runner runner;
	std::unique_ptr<torasu::ExecutionInterface> ei(runner.createInterface());


	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	RenderContext rctx;

	RenderResult* rr = rib.runRender(&tree, &rctx, ei.get());

	auto result = handle.getFrom(rr);
	cout << "DPNum Value: " << result.getResult()->getNum() << endl;
}

void slotFunction(
		torasu::tools::ElementSlot elemA, torasu::tools::ElementSlot elemB, torasu::tools::ElementSlot elemC,
		torasu::tools::RenderableSlot rndD, torasu::tools::RenderableSlot rndE, torasu::tools::RenderableSlot rndF) {
	torasu::tools::ManagedElementSlot mA(elemA);
	torasu::tools::ManagedElementSlot mB(elemB);
	torasu::tools::ManagedElementSlot mC(elemC);
	torasu::tools::ManagedRenderableSlot mD(rndD);
	torasu::tools::ManagedRenderableSlot mE(rndE);
	torasu::tools::ManagedRenderableSlot mF(rndF);

}

const auto& IE = torasu::tools::inlineElement;
const auto& IR = torasu::tools::inlineRenderable;

void slotTests() {

	Renderable* rndA = new Rnum(1);

	Rnum rndB(1);

	slotFunction(rndA, &rndB, IE(new Rnum(10)),
				rndA, &rndB, IR(new Rnum(7)));

	delete rndA;

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

	mathExample();

	slotTests();

	// taskDistTest();

	return 0;
}
