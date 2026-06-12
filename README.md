# Ethernet Baremetal Driver (STM32H723)
by Bex Sawetrattanathumrong & Farhaan Khan

"we writing ethernet driver from scratch :)"

## Installing Prerequisites

First, install the ARM toolchain. If you are on a Unix system, simply use your package installer, for example in Ubuntu: `sudo apt-get install gcc-arm-none-eabi`. On Windows, you can download it from [here](https://developer.arm.com/downloads/-/gnu-rm) - you may have to add it to your PATH. Make sure it worked: `arm-none-eabi-gcc --version`

Next, install CMake. On Ubuntu: `sudo apt-get install cmake`. On Windows, download it from [here](https://cmake.org/download/). Again, you may have to add it to your PATH. Make sure it worked: `cmake --version`

Also, install Ninja. On Ubuntu: `sudo apt-get install ninja-build`. On Windows, download it from [here](https://github.com/ninja-build/ninja/releases). Add it to your PATH. Make sure it worked: `ninja --version`

*(Note for Windows users: You can also install GCC/Clang and Ninja via MSYS2 / MinGW64).*

Lastly, pull in external dependencies (submodules):
```bash
git submodule update --init --recursive
```

## Building

To build: If you are on Windows, use the `make.ps1` script in PowerShell. If on Linux / Unix, use the `make.sh` script.

The minimum parameters look like this: `./make.ps1 -t <preset>`.
For example, to build for our target board:
```powershell
./make.ps1 -t stm32h723
```
*(See `CMakePresets.json` for available presets).*

It's also possible to specify a target application rather than building all available apps (which is the default), by using the `-a` parameter:
```powershell
./make.ps1 -t stm32h723 -a your_app
```

For a clean build, add the `-c` flag:
```powershell
./make.ps1 -t stm32h723 -c
```

Builds are by default done in `Debug` mode, but `Release` mode can be selected with the `-r` parameter:
```powershell
./make.ps1 -t stm32h723 -r
```

## Running Tests

Build for native with:
```powershell
./make.ps1 -t native
```

Navigate to the build directory with:
```powershell
pushd build/native
```
Followed by `ctest` to run tests! You can return to your previous directory with `popd`.

## Debugging

To debug, make sure you have OpenOCD installed. On Ubuntu: `sudo apt-get install openocd`. On Windows, download it from [here](https://openocd.org/pages/getting-openocd.html). You may have to add it to your PATH.

Additionally, grab the `cortex-debug` extension for VS Code. There are reference `launch.json` files found in the repository already under `.vscode`.

## Developing

Install `clang-format` to auto-format your code.
- On Windows, try: `python -m pip install clang-format` or install it alongside your compiler toolchain.
- On Linux, try: `sudo apt install clang-format`.

In VS Code, you can go to Settings > Text Editor > Formatting > Format On Save to enable auto-formatting on save. 

