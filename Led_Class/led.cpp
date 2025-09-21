#include "led.h"

Task::Task() {
    if (!set_gpio_direction()) {
        std::cerr << "[ERROR] Failed to set GPIO27 direction to out\n";
    }
    if (!set_switch_direction()) {
        std::cerr << "[ERROR] Failed to set GPIO17 direction to in\n";
    }
}

bool Task::set_gpio_direction() {
    if (!gpio27_direction.is_open()) {
        std::cout << "[ERROR] opening GPIO27 direction - can't write\n";
        return false;
    }
    gpio27_direction.seekp(0);
    gpio27_direction << "out";
    gpio27_direction.flush();
    return true;
}

bool Task::set_gpio_high() {
    if (!gpio27_value.is_open()) {
        std::cout << "[ERROR] opening GPIO27 value - can't write\n";
        return false;
    }
    gpio27_value.seekp(0);
    gpio27_value << "1";
    gpio27_value.flush();
    return true;
}

bool Task::set_gpio_low() {
    if (!gpio27_value.is_open()) {
        std::cout << "[ERROR] opening GPIO27 value - can't write\n";
        return false;
    }
    gpio27_value.seekp(0);
    gpio27_value << "0";
    gpio27_value.flush();
    return true;
}

bool Task::get_gpio_value() {
    if (!gpio27_value.is_open()) {
        std::cout << "[ERROR] opening GPIO27 value - can't read\n";
        return false;
    }
    gpio27_value.clear();
    gpio27_value.seekg(0);

    char value = '0';
    gpio27_value >> value;
    return value == '1';
}

bool Task::set_switch_direction() {
    if (!gpio17_direction.is_open()) {
        std::cout << "[ERROR] opening GPIO17 direction - can't write\n";
        return false;
    }
    gpio17_direction.seekp(0);
    gpio17_direction << "in";
    gpio17_direction.flush();
    return true;
}

bool Task::get_switch_value() {
    if (!gpio17_value.is_open()) {
        std::cout << "[ERROR] opening GPIO17 value - can't read\n";
        return false;
    }
    gpio17_value.clear();
    gpio17_value.seekg(0);

    char value = '0';
    gpio17_value >> value;
    return value == '1';
}

bool Task::act_on_led() {
    if (get_switch_value()) {  // <-- use bool directly
        return set_gpio_high();
    } else {
        return set_gpio_low();
    }
}
