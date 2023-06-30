# Permission-tools

It was a school project. We create a program which changes permissions in the directory.
To be able to work on this project I need to use POSIX so it was really interesting.

More information on the project and assignment are located in the file assignment.ad.


## How to extend the codebase

Feel free to:

* create new `*.c` and `*.h` files in this directory,
* write test `*.c` files in the `tests` directory

NOTE: If you want to use `getopt_long`, uncomment the designated line in
CMakeLists.txt.

## How to build

You have two options:

### 1. Use the terminal

Run the following commands:

```bash
mkdir build
cd build
cmake ..
make
./checkperms
```

### 2. Use an editor

Some IDEs and editors support CMake natively (e.g. CLion, Visual Studio, etc.).
Other require an extension (such as Visual Studio Code).

NOTE: If you must use a Windows machine, use
[Windows Subsystem for Linux](https://www.fi.muni.cz/pb071/man/#wsl).
