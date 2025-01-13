#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

class SerialPort {
private:
#ifdef _WIN32
    HANDLE hSerial;
#else
    int fd;
#endif
    bool isOpen;

    void throwError(const std::string& message) {
#ifdef _WIN32
        throw std::runtime_error(message + " Error code: " + std::to_string(GetLastError()));
#else
        throw std::runtime_error(message + " Error: " + std::strerror(errno));
#endif
    }

public:
    SerialPort(const std::string& portName) : isOpen(false) {
#ifdef _WIN32
        hSerial = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hSerial == INVALID_HANDLE_VALUE) {
            throwError("Failed to open port " + portName);
        }
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            throwError("Failed to get port state for " + portName);
        }
        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            CloseHandle(hSerial);
            throwError("Failed to set port state for " + portName);
        }
#else
        fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
            throwError("Failed to open port " + portName);
        }
        struct termios tty;
        if (tcgetattr(fd, &tty) != 0) {
            close(fd);
            throwError("Failed to get port attributes for " + portName);
        }
        cfsetospeed(&tty, B9600);
        cfsetispeed(&tty, B9600);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~(PARENB | PARODD);
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            close(fd);
            throwError("Failed to set port attributes for " + portName);
        }
#endif
        isOpen = true;
    }

    ~SerialPort() {
        if (isOpen) {
#ifdef _WIN32
            CloseHandle(hSerial);
#else
            close(fd);
#endif
        }
    }

    void write(const std::string& data) {
        if (!isOpen) {
            throw std::runtime_error("Port is not open");
        }
#ifdef _WIN32
        DWORD bytesWritten;
        if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, nullptr)) {
            throwError("Failed to write to port");
        }
#else
        ssize_t bytesWritten = ::write(fd, data.c_str(), data.size());
        if (bytesWritten < 0) {
            throwError("Failed to write to port");
        }
#endif
    }

    std::string readUntil(char terminator) {
        if (!isOpen) {
            throw std::runtime_error("Port is not open");
        }
        std::string result;
        char buffer[1]; // Читаем по одному символу
#ifdef _WIN32
        DWORD bytesRead;
        while (true) {
            if (!ReadFile(hSerial, buffer, 1, &bytesRead, nullptr) || bytesRead == 0) {
                throwError("Failed to read from port");
            }
            result += buffer[0];
            if (buffer[0] == terminator) {
                break;
            }
        }
#else
        ssize_t bytesRead;
        while (true) {
            bytesRead = ::read(fd, buffer, 1);
            if (bytesRead < 0) {
                throwError("Failed to read from port");
            }
            if (bytesRead == 0) {
                break; // Конец данных
            }
            result += buffer[0];
            if (buffer[0] == terminator) {
                break;
            }
        }
#endif
        return result;
    }
};