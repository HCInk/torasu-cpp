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
#include <torasu/std/Rstring_file.hpp>
#include <torasu/std/Rjson_prop.hpp>
#include <torasu/std/Rnet_file.hpp>
#include <torasu/std/Dstring.hpp>
#include <torasu/std/EIcore_runner.hpp>
#include <torasu/std/LIcore_logger.hpp>
#include <torasu/std/Rsin.hpp>
#include <torasu/std/Radd.hpp>
#include <torasu/std/Rfallback.hpp>
#include <torasu/std/simple_render.hpp>
#include <torasu/std/Rlog_message.hpp>
#include <torasu/std/Rerror.hpp>

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

	torasu::tstd::LIcore_logger logger;
	torasu::LogInstruction li(&logger, LogLevel::DEBUG);
	Dstring resStr = torasu::tstd::renderString(&tree, nullptr, &li);

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

void logExample() {
	cout << "//" << endl
		 << "// Log Example" << endl
		 << "//" << endl;

	torasu::tstd::LIcore_logger core_logger;

	torasu::LogInterface& logger = core_logger;

	logger.log(LogLevel::TRACE, "Trace-Message");
	logger.log(LogLevel::DEBUG, "Debug-Message");
	logger.log(LogLevel::INFO, "Info-Message");
	logger.log(LogLevel::WARN, "Warn-Message");
	logger.log(LogLevel::ERROR, "Error-Message");
	logger.log(LogLevel::SERVERE_ERROR, "Servere-Error-Message");

}

void renderLogExample() {
	cout << "//" << endl
		 << "// Render Log Example" << endl
		 << "//" << endl;

	Rnum dummyContent(1);
	Rlog_message msgServError(LogLevel::SERVERE_ERROR, "Servere-Error-Message (Rlog_message-example)", &dummyContent);
	Rlog_message msgError(LogLevel::ERROR, "Error-Message (Rlog_message-example)", &msgServError);
	Rlog_message msgWarn(LogLevel::WARN, "Warn-Message (Rlog_message-example)", &msgError);
	Rlog_message msgInfo(LogLevel::INFO, "Info-Message (Rlog_message-example)", &msgWarn);
	Rlog_message msgDebug(LogLevel::DEBUG, "Debug-Message (Rlog_message-example)", &msgInfo);
	Rlog_message msgTrace(LogLevel::TRACE, "Trace-Message (Rlog_message-example)", &msgDebug);

	auto& tree = msgTrace;

	torasu::tstd::LIcore_logger logger;
	torasu::LogInstruction li(&logger, LogLevel::TRACE);

	torasu::tstd::renderNum(&tree, nullptr, &li);

}

void jsonParseFromStrExample() {
	cout << "//" << endl
		 << "// Json Parse From String Example" << endl
		 << "//" << endl;

	Rstring str("{\"test\":\"value\"}");

	Rstring_file sf(&str);

	Rjson_prop prop("test", &sf);

	torasu::tstd::LIcore_logger logger;
	torasu::LogInstruction li(&logger, LogLevel::DEBUG);
	auto string = torasu::tstd::renderString(&prop, nullptr, &li);

	std::cout << "Res: " << string.getString() << std::endl;

}

void renderErrorExample() {

	cout << "//" << endl
		 << "// Render Error Example" << endl
		 << "//" << endl;

	Rerror err("Example error");
	Rmultiply mulA(10, &err);
	Rmultiply mulB(&mulA, &err);
	Rmultiply mulC(5, &mulB);
	auto& tree = mulC;

	torasu::tstd::LIcore_logger logger;
	torasu::LogInstruction li(&logger, LogLevel::DEBUG, torasu::LogInstruction::OPT_RUNNER_BENCH | torasu::LogInstruction::OPT_RUNNER_BENCH_DETAILED);
	auto num = torasu::tstd::renderNum(&tree, nullptr, &li);

	std::cout << "Res: " << num.getNum() << std::endl;

}

void progressExample() {

	cout << "//" << endl
		 << "// Progress Example" << endl
		 << "//" << endl;

	std::chrono::milliseconds initTime(500);
	std::chrono::milliseconds dummyTime(100);

	LIcore_logger logger;

	logger.log(new LogProgress( -1, 0, "Initializing..." ));
	std::this_thread::sleep_for(initTime);


	size_t total = 50;

	for (size_t i = 0; i < total; i++) {
		logger.log(new LogProgress( total, i, "Doing " + std::to_string(i+1) + "/" + std::to_string(total) + "..." ));
		std::this_thread::sleep_for(dummyTime);
	}

	logger.log(new LogProgress( total, total, "Finishing..." ));
	std::this_thread::sleep_for(dummyTime);

}

} // namespace torasu::texample

using namespace torasu::texample;

int main(int argc, char** argv) {

	checkLinkings();

	// boilerplate_execution_initializer();

	// simpleDpTest();

	simpleRenderExample1();

	simpleRenderExample2();

	// jsonPropExample();

	// jsonFallbackExample();

	mathExample();

	inlineMathExample();

	slotTests();

	// logExample();

	// renderLogExample();

	// jsonParseFromStrExample();

	// renderErrorExample();

	progressExample();

	// taskDistTest();

	return 0;
}
