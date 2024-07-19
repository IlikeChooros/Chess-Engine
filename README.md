# Chess-Engine
Chess engine made in cpp

# Setup


## Required libs

- List:
  - ```libudev-dev```
  - ```libalut-dev```
  - ```libvorbis-dev```
  - ```libx11-dev libxrandr-dev libxcursor-dev```
  - ```libfreetype-dev```
  - ```libjpeg-dev```
  - ```libx11-xcb-dev libxcb-randr0-dev libxcb-image0-dev```
  - ```freeglut3-dev```
  - ```libogg-dev```
  - ```libvorbisenc2```
  - ```libevent-pthreads-2.1-7```

Install them in one command:

```sh

sudo apt install libudev-dev libalut-dev libx11-dev libxrandr-dev libxcursor-dev libfreetype-dev libjpeg-dev libx11-xcb-dev libxcb-randr0-dev libxcb-image0-dev freeglut3-dev libogg-dev libvorbisenc2 libevent-pthreads-2.1-7

```

## Run project

In project directory run *(where `main.cpp` file is)*:

```sh
cmake -S . -B ./build
cmake --build build
make -C build
./build/bin/ChessEngine
```

## Test

First build the project, only then run the tests.

```sh
cd build
ctest
```

