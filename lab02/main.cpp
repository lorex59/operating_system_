#include <string>
#include <iostream>
#include <thread>
#include "lib.cpp"
int main() {
#ifdef _WIN32
    string subprogram_ok = "./subprogram_ok.exe";
    string subprogram_err = "./subprogram_err.exe";
#else
    string subprogram_ok = "./subprogram_ok";
    string subprogram_err = "./subprogram_err";
#endif
    int exit_code;

    cout << "Running ok background async" << endl;
    run_async(subprogram_ok);

    cout << "Running ok background sync" << endl;
    exit_code = run_sync(subprogram_ok);
    cout << "Program exited with code: " << exit_code << endl;


    cout << "Running err background sync" << endl;
    exit_code = run_sync(subprogram_err);
    cout << "Program exited with code: " << exit_code << endl;

    return 0;
}
