#include <iostream>
using namespace std;

int TORASU_check_core();
int TORASU_check_std();

int main(int argc, char **argv) {
    cout << "Checking core..." << endl;
	TORASU_check_core();
    cout << "Checking std..." << endl;
	TORASU_check_std();
    cout << "Everything seems good!" << endl;
	return 0;
}
