#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

using namespace std;
// Функция для синхронного запуска программы в фоне (ждём код ответа)
int run_sync(const string& program) {
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    string cmd = program;

    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        cerr << "Failed to start program: " << program << endl;
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code;
    if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
        cerr << "Failed to get exit code for program: " << program << endl;
        exit_code = -1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(exit_code);
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Ребёнок
        execlp(program.c_str(), program.c_str(), nullptr);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Родитель
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork");
        return -1;
    }
#endif
}

// Функция для асинхронного запуска программы (не дожидаемся кода ответа)
void run_async(const string& program) {
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    string cmd = program;

    if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        cerr << "Failed to start program: " << program << endl;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Ребёнок
        execlp(program.c_str(), program.c_str(), nullptr);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Родитель ничего не ждёт
        return;
    } else {
        perror("fork");
    }
#endif
}

