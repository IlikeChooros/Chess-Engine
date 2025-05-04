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

You may also play with the `--ui` option to start the engine in a UI mode.
(right now it is a console UI and you can only play against the engine)

```sh
./build/bin/ChessEngine --ui
```

You can also specify ahead `--fen`, `--side`, and `--limits` options to set the initial position, side of the player, and engine limits respectively, all of which are optional.

For example:

```sh
./build/bin/ChessEngine --ui --fen startpos --side w --limits "movetime 1000"
```

Will result in a starting position, where your side is white, and the engine will have a time limit of 1 second.

See the uci protocol for more details on these options.


## Test

First build the project, only then run the tests.

```sh
cd build && ctest
```

