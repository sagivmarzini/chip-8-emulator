# CHIP-8 Emulator in C (SDL3)

A lightweight, high-performance CHIP-8 interpreter written in C using **SDL3** for graphic rendering, input handling,
and audio generation.

## Features

* **Full CPU Emulation:** Complete instruction set implementation with accurate clock cycling (~500Hz execution rate).
* **SDL3 Graphics:** Clean, green monochrome display grid scaling up the original 64x32 space into a modern 640x320
  window.
* **Audio Engine:** Dynamic square wave audio synthesis using the updated SDL3 audio stream pipeline.
* **Classic ROM Support:** Built-in loader optimized for handling traditional CHIP-8 application boundaries.

---

## Keypad Mapping

The classic 16-key hexadecimal CHIP-8 keypad is mapped directly to a standard modern QWERTY layout.

```text
Classic CHIP-8 Keypad:          Modern Keyboard Mapping:
┌───┬───┬───┬───┐               ┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ C │               │ 1 │ 2 │ 3 │ 4 │
├───┼───┼───┼───┤               ├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ D │     ───►      │ Q │ W │ E │ R │
├───┼───┼───┼───┤               ├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ E │               │ A │ S │ D │ F │
├───┼───┼───┼───┤               ├───┼───┼───┼───┤
│ A │ 0 │ B │ F │               │ Z │ X │ C │ V │
└───┴───┴───┴───┘               └───┴───┴───┴───┘

```

* Press `ESC` at any time to instantly exit the emulator.

---

## Prerequisites & Installation

### 1. Install SDL3

Ensure you have the development libraries for SDL3 installed on your machine.

* **macOS (Homebrew):**

```bash
brew install sdl3
```

* **Linux (Ubuntu/Debian):**

```bash
sudo apt install libsdl3-dev
```

### 2. Build the Project

This project uses CMake. Follow these steps to generate the build files and compile:

```bash
# Create and navigate to build directory
mkdir build && cd build

# Generate build files
cmake ..

# Compile the target binary
cmake --build .
```

---

## Running Games

When you launch the emulator, it will automatically scan the `../games` directory and present an interactive menu in
your console terminal.

Simply enter the number corresponding to the game you want to play:

```text
=== CHIP-8 ROM SELECTOR ===
[1] astro.ch8
[2] bowling.ch8
[3] breakout.ch8
[4] connect-4.ch8
[5] pong-1.ch8
[6] tetris.ch8

Select a game number (1-6): 5
Loading: ../games/pong-1.ch8