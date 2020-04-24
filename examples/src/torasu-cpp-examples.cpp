// System
#include <iostream>

// Dev Dependencies
#include <nlohmann/json.hpp>

// TORASU CORE
#include <torasu/torasu.h>
#include <torasu/DataPackable.h>

// TORASU STD
#include <torasu/std/torasu_std.h>

using namespace std;
using namespace torasu;

int TORASU_check_std();

int main(int argc, char **argv) {
    cout << "Checking core..." << endl;
	TORASU_check_core();
    cout << "Checking std..." << endl;
	TORASU_check_std();
    cout << "Everything seems good!" << endl;

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

	return 0;
}
