## Requirements
- CMake 3.10 or higher
- A C++ compiler (e.g., GCC, Clang, MSVC)
- Virtual COM port drivers:
  - Windows: [com0com](https://sourceforge.net/projects/com0com/)
  - Linux: [socat](http://www.dest-unreach.org/socat/)

## Build
1. Generate the build files using CMake:
    ```sh
    cmake .
    ```

2. Compile the project:
    ```sh
    make
    ```


## Run
To run the `writer` executable that can emulate the com device:
- On Windows:
    ```sh
    writer.exe
    ```
- On Linux:
    ```sh
    ./writer
    ```

Then, run the temperature-logger:
- On Windows:
    ```sh
    main.exe
    ```
- On Linux:
    ```sh
    ./main
    ```