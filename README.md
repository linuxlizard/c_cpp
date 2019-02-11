# C/C\+\+
Re-learning C++. Doing a few projects with WiFi and REST APIs. Uses CMake, C\+\+17, [Boost](https://www.boost.org), and [Microsoft's C++ REST SDK](https://github.com/Microsoft/cpprestsdk)

# Dependencies
## Debian based (Ubuntu, Kali)
The dependencies of the dev packages usually drags in everything else required (the rest of Boost, for example.)  TODO need to put together a list of packages *just* for run-time support.
```sh
apt install build-essentials cmake libcpprest-dev libboost-program-options-dev libboost-test-dev
```
## Fedora
```sh
dnf install @development-tools cmake gcc-c++ cpprest-devel zlib-devel openssl-devel boost-devel 
```
# Building
```sh
mkdir build
cd build
cmake ..
make
make test  # run the boost-test tests
