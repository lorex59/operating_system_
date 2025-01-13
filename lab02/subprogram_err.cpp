#include <thread>
#include <iostream>
using namespace std;
int main() {
    cout << "Hello from subprogram with error!\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "Subprogram with error ended with code 1!\n";
    exit(1);
}