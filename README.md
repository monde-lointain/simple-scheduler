# Embedded Artistry libc

This program demonstrates a basic cooperative round-robin scheduler made for the STM32 F4 Discovery board (STM32F407G-DISC1).

[![Demonstration video](http://img.youtube.com/vi/KnVh6RiTZRk/0.jpg)](https://www.youtube.com/shorts/KnVh6RiTZRk "STM32F4 Round-Robin Scheduler Demo")

## Building

Ensure you have CMake installed on your computer, then run the following commands:

```bash
git clone https://github.com/monde-lointain/simple-scheduler.git
cd simple-scheduler
mkdir build && cd build
cmake ..
cmake --build .
```