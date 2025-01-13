#include <string>
#include <random>
#include <thread>
#include <chrono>

#include "port.cpp"
using namespace std;

int main() {
    string port_name;
#ifdef _WIN32  
    port_name = "COM1";
#else
    port_name = "/dev/pts/3";
#endif

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-20, 20);
    SerialPort port(port_name);

    int temperature;

    while(1) {
        // Генерация случайной температуры
        temperature = dis(gen);
        
        port.write(to_string(temperature) + '\n');
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}