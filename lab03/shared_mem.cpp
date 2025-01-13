#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#endif

class SharedMemory {
private:
    int* shared_value;
#ifdef _WIN32
    HANDLE hMapFile;
    HANDLE hSemaphore;
#else
    int shm_fd;
    sem_t* semaphore;
#endif

    const std::string shm_name = "/lorex_memory";
    const std::string sem_name = "/lorex_semaphore";

public:
    SharedMemory() {
#ifdef _WIN32
        hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(int),shm_name.c_str());
        if (!hMapFile) {
            throw std::runtime_error("Failed to create file mapping.");
        }
        shared_value = static_cast<int*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int)));
        if (!shared_value) {
            CloseHandle(hMapFile);
            throw std::runtime_error("Failed to map view of file.");
        }

        hSemaphore = CreateSemaphoreA(nullptr, 1, 1, sem_name.c_str());
        if (!hSemaphore) {
            UnmapViewOfFile(shared_value);
            CloseHandle(hMapFile);
            throw std::runtime_error("Failed to create semaphore.");
        }
#else
        shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            throw std::runtime_error("Failed to open shared memory.");
        }
        if (ftruncate(shm_fd, sizeof(int)) == -1) {
            close(shm_fd);
            throw std::runtime_error("Failed to resize shared memory.");
        }
        shared_value = static_cast<int*>(mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
        if (shared_value == MAP_FAILED) {
            close(shm_fd);
            throw std::runtime_error("Failed to map shared memory.");
        }

        semaphore = sem_open(sem_name.c_str(), O_CREAT, 0666, 1);
        if (semaphore == SEM_FAILED) {
            munmap(shared_value, sizeof(int));
            close(shm_fd);
            throw std::runtime_error("Failed to create semaphore.");
        }
#endif
    }

    ~SharedMemory() {
#ifdef _WIN32
        UnmapViewOfFile(shared_value);
        CloseHandle(hMapFile);
        CloseHandle(hSemaphore);
#else
        munmap(shared_value, sizeof(int));
        close(shm_fd);
        sem_close(semaphore);
        sem_unlink(sem_name.c_str());
        shm_unlink(shm_name.c_str());
#endif
    }

    void lock() {
#ifdef _WIN32
        WaitForSingleObject(hSemaphore, INFINITE);
#else
        sem_wait(semaphore);
#endif
    }

    void unlock() {
#ifdef _WIN32
        ReleaseSemaphore(hSemaphore, 1, nullptr);
#else
        sem_post(semaphore);
#endif
    }

    int get() {
        int value = *shared_value;
        return value;
    }

    void set(int value) {
        *shared_value = value;
    }

    void increment() {
        ++(*shared_value);
    }
};