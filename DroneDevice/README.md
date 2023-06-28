Installation
------------

1. Required packages:

CMake 3.7 or newer
gcc 5.4 or newer

2. Clone project:

```sh
git clone git@gitlab.corp.geoscan.aero:uav-devices/DroneDevice.git
cd DroneDevice
git submodule update --init --recursive
```

3. Make tests on x86 platforms:

```sh
mkdir build_x86
cd build_x86
cmake ..
make
make test
```
