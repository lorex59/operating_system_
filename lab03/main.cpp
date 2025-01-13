#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>

#include "shared_mem.cpp"
#include "primary.cpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;


void write_to_log_file(const string& str) {
    ofstream log("./log_file.txt", ios::app);
    log << str << endl;
}

string add_log_data_to_string(const string& str) {
    int process_id = 
#ifdef _WIN32
        GetCurrentProcessId();
#else
        getpid();
#endif

    auto now = chrono::system_clock::now();
    auto time_t_now = chrono::system_clock::to_time_t(now);
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    ostringstream oss;
    oss << "(pid:" << process_id 
        << " | " << put_time(localtime(&time_t_now), "%Y-%m-%d %H:%M:%S")
        << "." << setfill('0') << setw(3) << milliseconds.count() 
        << ") " << str;

    return oss.str();
}


void log_thread(SharedMemory& shmem, atomic<bool>& stop_flag) {
    while(!stop_flag.load()) {
        shmem.lock();
        write_to_log_file(add_log_data_to_string("Current counter: " + to_string(shmem.get())));
        shmem.unlock();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void counter_val_increaser_thread(SharedMemory& shmem, atomic<bool>& stop_flag) {
    while(!stop_flag.load()) {
        shmem.lock();
        shmem.increment();
        shmem.unlock();
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}


bool spawn_program(const string& program, const string& flag) {
#ifdef _WIN32
        string command = program + " " + flag;
        STARTUPINFOA si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        if (!CreateProcessA(nullptr, (LPSTR)command.c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            cerr << "Error starting process: " << GetLastError() << endl;
            return false;
        }
#else
        auto pid = fork();
        if (pid == 0) {
            execlp(program.c_str(), program.c_str(), flag.c_str(), nullptr);
            cerr << "Error executing program" << endl;
            return false;
        } else if (pid < 0) {
            cerr << "Error forking process" << endl;
            return false;
        }
#endif
        return true;
    }



bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void copy_spawner_thread(const string& this_program, SharedMemory& shmem, atomic<bool>& stop_flag) {
    while (!stop_flag.load()) {
        this_thread::sleep_for(chrono::seconds(3));
        if (file_exists("copy1.tmp")) {
            write_to_log_file(add_log_data_to_string("Cannot spawn copy1 - already exists!" ));
            continue;
        }

        if (file_exists("copy2.tmp")) {
            write_to_log_file(add_log_data_to_string("Cannot spawn copy2 - already exists!" ));
            continue;
        }

        if (!spawn_program(this_program, "copy_1")) 
            write_to_log_file(add_log_data_to_string("Failed to spawn copy_1 program" ));  
        if (!spawn_program(this_program, "copy_2"))
            write_to_log_file(add_log_data_to_string("Failed to spawn copy_2 program"));    
    }
}

void check_is_copy(int argc, char* argv[], SharedMemory& shmem) {
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "copy_1") {
            SingleInstance instance("copy_1.tmp");
            if (!instance.isPrimary()) 
                exit(0);
            
            write_to_log_file(add_log_data_to_string("Process Copy 1 Started"));
            shmem.lock();
            shmem.set(shmem.get() + 10);
            shmem.unlock();
            write_to_log_file(add_log_data_to_string("Process Copy 1 Ended"));
            instance.~SingleInstance();
            exit(0);
        } else if (arg == "copy_2") {
            SingleInstance instance("copy_2.tmp");
            if (!instance.isPrimary())
                exit(0);

            write_to_log_file(add_log_data_to_string("Process Copy 2 Started"));
            shmem.lock();
            shmem.set(shmem.get() * 2);
            shmem.unlock();

            this_thread::sleep_for(chrono::seconds(2));

            shmem.lock();
            shmem.set(shmem.get() / 2);
            shmem.unlock();

            write_to_log_file(add_log_data_to_string("Process Copy 2 Ended"));
            instance.~SingleInstance();
            exit(0);
        }
    }

}
int main(int argc, char* argv[]) {
    SingleInstance instance("primary_program.tmp");
    SharedMemory shmem;
    atomic<bool> stop_flag(false);
    thread log, counter_increaser, copy;

    check_is_copy(argc, argv, shmem);

    write_to_log_file(add_log_data_to_string("Process Started"));
    // Только 1 программа может писать логи и делать копии
    if (instance.isPrimary()) {
        log = thread(log_thread, ref(shmem), ref(stop_flag));
        copy = thread(copy_spawner_thread, argv[0], ref(shmem), ref(stop_flag));
    }

    counter_increaser = thread(counter_val_increaser_thread, ref(shmem), ref(stop_flag));

    string input;
    while(true) {
        cout << "Enter a number to update the counter or 'quit': ";
        cin >> input;
        if (input == "quit" || input == "q") {
            break;
        }
        try {
            int value = stoi(input);
            shmem.lock();
            shmem.set(value);
            shmem.unlock();
        } catch (invalid_argument& e) {
            cout << "Invalid input. Please enter a valid number or 'quit' to exit." << endl;
        }
    }

    stop_flag.store(true);
    cout << "Stopping\n";

    if (instance.isPrimary()) {
        log.join();
        copy.join();
    }

    counter_increaser.join();
}