TouchJoy - On-screen gamepad for touch-screen Windows devices
=============================================================

## What is it?

This program creates a highly customizable on-screen virtual gamepad so you can play modern PC games on a touch-enabled Windows device (like a Microsoft Surface, ROG Ally, or Windows tablet).

![screenshot](data/screenshot.jpg)

Unlike legacy mappers that just send fake keyboard presses, **TouchJoy natively emulates an Xbox 360 Controller**. This means it is instantly recognized by modern games via XInput with full analog joystick and trigger support.

The layout is fully customizable using a simple `config.ini` file. The file is automatically reloaded whenever it is saved, so you can tweak and adjust your layout in real-time without restarting the app.

## Prerequisites

Because TouchJoy emulates a physical hardware controller, it requires the **ViGEmBus** kernel-mode driver to be installed on your system.

* If you do not have it installed, TouchJoy will automatically prompt you on startup and open your browser to the [Official ViGEmBus GitHub Releases](https://github.com/nefarius/ViGEmBus/releases/latest) page so you can download it safely.

## How to install and use?

**The easy way:**
1. Go to the **Actions** or **Releases** tab on this GitHub repository.
2. Download the latest `TouchJoy-Windows.zip` artifact.
3. Extract the folder anywhere on your PC.
4. Double click `TouchJoy.exe`. 
5. To change your layout or map different buttons, open `config.ini` in any text editor. (For now, the code and `config.ini` itself are the best manuals).

## How to build from source?

The project has been modernized to use **CMake**. It requires Visual Studio 2019/2022 (with the Desktop C++ workload) or any modern C99-compliant compiler.

```bash
# Clone the repository
git clone [https://github.com/YOUR-USERNAME/TouchJoy.git](https://github.com/YOUR-USERNAME/TouchJoy.git)
cd TouchJoy

# Configure the build system (this will automatically fetch ViGEmClient)
cmake -B build -S .

# Compile the project
cmake --build build --config Release
