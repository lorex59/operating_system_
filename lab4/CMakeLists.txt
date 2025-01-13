cmake_minimum_required(VERSION 3.10)

project(SerialPortProject)

add_executable(writer writer.cpp port.cpp)
add_executable(main main.cpp port.cpp)