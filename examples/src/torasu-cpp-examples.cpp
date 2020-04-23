#include <iostream>

#include <nlohmann/json.hpp>

#include "../../core/src/torasu.h"
#include "../../core/src/DataPackable.h"

using namespace std;
using namespace torasu;

int TORASU_check_core();
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

    DataDump dump = dpu.getData();

    if (dump.getFormat() == DDDataPointerType::DDDataPointerType_JSON_CSTR) {
        std::cout << "data:" << dump.getData().s << std::endl;
    } else {
        std::cerr << "unexpected DDDPT" << std::endl;
    }

	return 0;
}
