#include <thread>
#include <iostream>
using namespace std;
int main() {
    cout << "Hello from subprogram!\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "Subprogram ended!\n";
    exit(0);
}