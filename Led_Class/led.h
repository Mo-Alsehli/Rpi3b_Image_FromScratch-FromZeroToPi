#pragma once
#include <fstream>
#include <iostream>
#include <string>

/*
* For my raspi I had to map the gpio pins with offset.
* There is a folder which called gpiochip512, it's appear that we can't access gpio pins
    directly with it's corresponding numbers.
* So we use an offset of it and the base is 512.
* if we need to export for ex gpio1 the number of it will be (512 + 1 <offset>).
* so here we need to control (gpio17 - gpio27) the corresponding number for them will be (529 - 539).
*/
class LedGpio27Pin {
   private:
    const int offset = 512;
    const std::string gpio27_path = "/sys/class/gpio/gpio" + std::to_string((27 + offset)) + "/";

   protected:
    std::fstream gpio27_direction;
    std::fstream gpio27_value;

   public:
    LedGpio27Pin() {
        gpio27_value.open((gpio27_path + "value"), std::ios::in | std::ios::out);
        gpio27_direction.open((gpio27_path + "direction"), std::ios::in | std::ios::out);
    }
    virtual bool set_gpio_direction() = 0;
    virtual bool set_gpio_high() = 0;
    virtual bool set_gpio_low() = 0;
    virtual bool get_gpio_value() = 0;
    ~LedGpio27Pin() {
        if (gpio27_value.is_open()) gpio27_value.close();
        if (gpio27_direction.is_open()) gpio27_direction.close();
    }
};

class SwitchGpio17Pin {
   private:
    const int offset = 512;
    const std::string gpio17_path = "/sys/class/gpio/gpio" + std::to_string((17 + offset)) + "/";

   protected:
    std::fstream gpio17_value;
    std::fstream gpio17_direction;

   public:
    SwitchGpio17Pin() {
        gpio17_value.open((gpio17_path + "value"), std::ios::in | std::ios::out);
        gpio17_direction.open((gpio17_path + "direction"), std::ios::in | std::ios::out);
    }
    virtual bool set_switch_direction() = 0;
    virtual bool get_switch_value() = 0;
    ~SwitchGpio17Pin() {
        if (gpio17_value.is_open()) gpio17_value.close();
        if (gpio17_direction.is_open()) gpio17_direction.close();
    }
};

class Task : public LedGpio27Pin, public SwitchGpio17Pin {
   private:
    bool led_state{};
    bool switch_state{};

   public:
    Task();
    bool set_gpio_direction() override;
    bool set_gpio_high() override;
    bool set_gpio_low() override;
    bool get_gpio_value() override;
    bool set_switch_direction() override;
    bool get_switch_value() override;
    bool act_on_led();
};
