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
#include <torasu/std/LIcore_logger.hpp>
#include <torasu/std/Rsin.hpp>
#include <torasu/std/Radd.hpp>
#include <torasu/std/Rfallback.hpp>
#include <torasu/std/simple_render.hpp>

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
	LIcore_logger logger;
	LogInstruction li(&logger);

	ExecutionInterface* ei = runner.createInterface();

	// Creating instruction

	tools::RenderInstructionBuilder rib;

	auto handle = rib.addSegmentWithHandle<Dnum>("STD::PNUM", NULL);

	// Running render based on instruction

	RenderContext rctx;

	RenderResult* rr = rib.runRender(&tree, &rctx, ei, li);

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

	Rsubtract tree(&numA, &numB);

	Dnum result = torasu::tstd::renderNum(&tree);

	std::cout << "Result: " << std::to_string(result.getNum()) << std::endl;

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

	Dstring result = torasu::tstd::renderString(&tree);

	std::cout << "Result: \"" << result.getString() << "\"" << std::endl;
}


void jsonFallbackExample() {

	//
	// Json Fallback Example
	//

	cout << "//" << endl
		 << "// Json Fallback Example" << endl
		 << "//" << endl;

	torasu::tstd::Rnet_file jsonFile("https://dummy.restapiexample.com/api/v1/employee/2");
	torasu::tstd::Rjson_prop json1("data.employee_namae", &jsonFile, true);
	torasu::tstd::Rjson_prop json2("data.employee_name", &jsonFile);

	torasu::tstd::Rfallback tree({&json1, &json2});

	Dstring resStr = torasu::tstd::renderString(&tree);

	std::cout << "EXEC-RES: \"" << resStr.getString() << "\"" << std::endl;

}

void mathExample() {

	cout << "//" << endl
		 << "// Math Example" << endl
		 << "//" << endl;

	Rnum num(10);
	Rsin sin(&num);

	auto& tree = sin;


	Dnum result = torasu::tstd::renderNum(&tree);

	std::cout << "Result: " << std::to_string(result.getNum()) << std::endl;
}


void inlineMathExample() {

	cout << "//" << endl
		 << "// Inline Math Example" << endl
		 << "//" << endl;

	Rnum num(10);
	Radd add(2, &num);
	Rsin sin(&add);

	auto& tree = sin;

	Dnum result = torasu::tstd::renderNum(&tree);

	std::cout << "Result: " << std::to_string(result.getNum()) << std::endl;
}

void slotFunction(
	torasu::tools::ElementSlot elemA, torasu::tools::ElementSlot elemB, torasu::tools::ElementSlot elemC,
	torasu::tools::RenderableSlot rndD, torasu::tools::RenderableSlot rndE, torasu::tools::RenderableSlot rndF,
	torasu::tstd::NumSlot rndNumA, torasu::tstd::NumSlot rndNumB, torasu::tstd::NumSlot rndNumC,
	torasu::tstd::StringSlot rndStrA, torasu::tstd::StringSlot rndStrB, torasu::tstd::StringSlot rndStrC) {
	torasu::tools::ManagedElementSlot mA(elemA);
	torasu::tools::ManagedElementSlot mB(elemB);
	torasu::tools::ManagedElementSlot mC(elemC);
	torasu::tools::ManagedRenderableSlot mD(rndD);
	torasu::tools::ManagedRenderableSlot mE(rndE);
	torasu::tools::ManagedRenderableSlot mF(rndF);
	torasu::tools::ManagedSlot<NumSlot> mG(rndNumA);
	torasu::tools::ManagedRenderableSlot mH(rndNumB);
	torasu::tools::ManagedSlot<NumSlot> mI(rndNumC);
	torasu::tools::ManagedRenderableSlot mJ(rndStrA);
	torasu::tools::ManagedRenderableSlot mK(rndStrB);
	torasu::tools::ManagedSlot<StringSlot> mL(rndStrC);
	// Also works but will not be optimal: torasu::tools::ManagedSlot<NumSlot> mL(rndStrC);

}

const auto& IE = torasu::tools::inlineElement;
const auto& IR = torasu::tools::inlineRenderable;

void slotTests() {

	Renderable* rndA = new Rnum(1);

	Rnum rndB(1);
	Rstring rndC("TEST2");

	slotFunction(rndA, &rndB, IE(new Rnum(10)),
				 rndA, &rndB, IR(new Rnum(7)),
				 5, &rndB, IR(new Rnum(6)),
				 "TEST1", &rndC, IR(new Rstring("TEST3")));

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

	jsonFallbackExample();

	mathExample();

	inlineMathExample();

	slotTests();

	// taskDistTest();

	return 0;
}
