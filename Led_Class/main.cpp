#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "led.h"

int main() {
    std::string state;

    // std::fstream led_gpio27_value("/sys/class/gpio/gpio539/value", std::ios::in | std::ios::out);
    // std::fstream switch_gpio17_value("/sys/class/gpio/gpio529/value", std::ios::in | std::ios::out);
    // std::fstream led_gpio27_direction("/sys/class/gpio/gpio539/direction", std::ios::in | std::ios::out);
    // std::fstream switch_gpio17_direction("/sys/class/gpio/gpio529/direction", std::ios::in | std::ios::out);

    Task task;

    std::cout << "<================ Control LED App ================>\n";
    std::cout << "Usage: \n";
    std::cout << "Press the Hard switch to light up led on gpio pin 27";

    while (true) {
        if (task.get_switch_value()) {
            task.set_gpio_high();
            std::cout << "Switch ON -> LED ON\n";
        } else {
            task.set_gpio_low();
            std::cout << "Switch OFF -> LED OFF\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "[INFO] Exiting LED Control App...\n";
    return 0;
}
