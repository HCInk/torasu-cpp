// Dev Dependencies
#include <nlohmann/json.hpp>

// TORASU CORE
#include <torasu/torasu.hpp>
#include <torasu/DataPackable.hpp>

// TORASU STD
#include <torasu/std/torasu_std.hpp>
#include <torasu/std/RNum.hpp>
#include <torasu/std/DPNum.hpp>

// System
#include <iostream>

using namespace std;
using namespace torasu;
using namespace torasustd;


namespace torasuexamples {

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

    std::string segKey = // Segement key the result will be saved to 
		std::string("x");

    ResultSegmentSettings* rss = // Save a segment from the pipeline "STD::PNUM" at the segemnt-key "x"
		new ResultSegmentSettings(std::string("STD::PNUM"), segKey, NULL);

    ResultSettings* rs = // Create a ResultSettings-object to put the ResultSegmentSettings in
		new ResultSettings();

    rs->push_back(rss); // Add the previously defined ResultSegmentSettings intot the ResultSettings-object

    RenderInstruction* ri = // Save the ResultInstruction with the defined ResultSettings and the RenderContext (which is unset/NULL for now)
		new RenderInstruction(NULL, rs);

    // Running render based on instruction

    RenderResult rr = rnum.render(ri);
    
    // Finding results

    map<string, ResultSegment*>* results = rr.getResults();

    map<string, ResultSegment*>::iterator found = results->find(segKey);

    if (found != results->end()) {
        cout << "Found result for segKey \"" + segKey + "\"" << endl;
        ResultSegment* rs = found->second;
        DataResource* dr = rs->getResult();

        if(DPNum* dpn = dynamic_cast<DPNum*>(dr)) {
            cout << "DPNum Value: " << dpn->getNum() << endl;
        } else {
            cerr << "Returned object is not DPNum!" << endl;
        }

    } else {
        cerr << "No result found for segKey \"" + segKey + "\"" << endl;
    }

    // Cleaning
	
    for (pair<string, ResultSegment*> res : *results) {
        delete res.second;
    }

    delete results;
    delete ri;
    delete rs;
    delete rss;
}

}

using namespace torasuexamples;

int main(int argc, char **argv) {

    checkLinkings();

    simpleDpTest();

    simpleRenderExample();

	return 0;
}
