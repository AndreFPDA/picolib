# Environment Preparation for RaspBerry Pico

## 1. Prepare the environment

All references for environment configuration can be found in the following link:

https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html

### 1.1 Prepare the Linux environment - Summary

To prepare the development environment, follow these steps:

- Create the `~/pico` directory:

```
cd ~/
mkdir pico
cd pico
```

- Clone the SDK and examples:

```
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
cd ..
git clone https://github.com/raspberrypi/pico-examples.git --branch master
```

- Install the Toolchain

```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
```
## 2. Preparing the development board:

Follow the link for detailed information on the procedure for connecting the reset button:
https://embarcados.com.br/programacao-da-raspberry-pi-pico/

- To connect the reset button, connect the button terminals to the PIN30(RUN) and GND pins.

## 3. Setting up the development environment for this project - Summary:

To set up the development environment, follow these steps:

- Clone this project into the directory:

```
cd ~/pico
git clone https://github.com/AndreFPDA/picolib.git
```

- Within this project's directory, the `build` directory must be created;

```
cd picolib/src/
mkdir build; && cd build;
```

- The following environment variable must be executed in the terminal inside the `~/pico/picolib/src/build` directory:

[!NOTE] ATTENTION: The environment variable export and CMAKE must be executed only once the terminal is opened.

```
export PICO_SDK_PATH=../../../pico-sdk
```

```
cmake ..
```

### 3.1 Command to compile:
- Compile with one thread:
```
make
```

- Compile with 4 threads:
```
make -j4
```

- Clean compiled files:
```
make clean
```

### 3.2 Load code:

- You must start the Rasp Pico in boot mode, to do this, simply connect or reset the Rasp Pico by pressing the boot button on the board.

- The Rasp Pico will be recognized as an external media by Linux.

- Then just copy the file `main.uf2`, which is located in `~/pico/picolib/build`, into the pico:

```
cp ~/pico/picolib/build/main.uf2 /media/<username>/RPI-RP2
```

### 3.3 Serial monitor:

- Putty - Recommended:

Putty is a monitor used to test various types of communication. It can be used as a serial monitor.

It is available for Linux and Windows.

It can be downloaded from the following link: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html

## 4. Editing the code

### 4.2 Clang Format - To keep the code organized in a standardized manner - MANDATORY.

- Installing Clang Format 11:

```
sudo apt install clang-format-11
```

- To format *.hpp and *.cpp files, for example:

```
clang-format-11 -i file_name_format.cpp
```

### 4.3 VSCode Environment - Recommended

- Linux packages that must be installed beforehand:

- VSCode applications:
- C/C++ v1.16.3 -> Microsoft
- C/C++ Extension Pack-> Microsoft
- Clang-Format -> Xaver Hellauer
- GitLens -> GitLens

To avoid problems with accents and formats:
- In VSCode, Settings, Text Editor, Code Actions On Save, click on “Edit in settings.json”.
- To access settings.json, use the shortcut command <Ctrl+Shift+P> and search for the file name.
- Copy and paste the following settings:
```
{
"files.encoding": "UTF-8",
"terminal.integrated.persistentSessionScrollback": 100000000,
"terminal.integrated.scrollback": 100000000,
"codegnuglobal.autoupdate": false,
"editor.defaultFormatter": "xaver.clang-format",
"clang-format.assumeFilename": ".clang-format",
"editor.formatOnSave": true,
"clang-format.executable": "clang-format-11",
"window.zoomLevel": -1
}
```
