# GPIO LED Control Application

A C++ application for Raspberry Pi that controls an LED based on a hardware switch input using Linux GPIO sysfs interface.

## Project Overview

This application demonstrates basic GPIO operations on Raspberry Pi by:
- Reading input from a hardware switch connected to GPIO17
- Controlling an LED connected to GPIO27 based on switch state
- Using Linux sysfs interface for GPIO access

## Hardware Requirements

- Raspberry Pi (tested on Pi 3)
- LED and current-limiting resistor (220Ω recommended)
- Hardware switch/button
- Breadboard and jumper wires

## Circuit Connection

```
GPIO17 (Pin 11) ──── Switch ──── 3.3V (Pin 1)
GPIO27 (Pin 13) ──── LED ──── Resistor ──── GND (Pin 6)
```

**Note**: This implementation uses GPIO offset mapping where:
- GPIO17 maps to `/sys/class/gpio/gpio529/`
- GPIO27 maps to `/sys/class/gpio/gpio539/`
- Base offset: 512

## File Structure

```
gpio-led-control/
├── main.cpp          # Main application loop
├── led.h             # Class definitions and GPIO interfaces
├── led.cpp           # GPIO control implementation
├── Makefile          # Build configuration
└── README.md         # This file
```

## Code Architecture

### Class Hierarchy

- **LedGpio27Pin**: Abstract base class for LED GPIO operations
- **SwitchGpio17Pin**: Abstract base class for switch GPIO operations  
- **Task**: Concrete implementation inheriting from both base classes

### Key Methods

```cpp
// GPIO27 (LED) control
bool set_gpio_direction();  // Set pin as output
bool set_gpio_high();       // Turn LED on
bool set_gpio_low();        // Turn LED off
bool get_gpio_value();      // Read current LED state

// GPIO17 (Switch) control
bool set_switch_direction(); // Set pin as input
bool get_switch_value();     // Read switch state

// Application logic
bool act_on_led();          // Control LED based on switch
```

## Building the Application

### Prerequisites
Ensure you have a cross-compiler for ARM64 if building on a host machine:
```bash
export CROSS_COMPILE=aarch64-linux-gnu-
```

### Compile
```bash
# On Raspberry Pi directly
g++ -std=c++11 -o led_control main.cpp led.cpp

# Cross-compile for Pi
aarch64-linux-gnu-g++ -std=c++11 -o led_control main.cpp led.cpp
```

## GPIO Setup

Before running the application, export the required GPIO pins:

```bash
# Export GPIO pins (adjust numbers based on your GPIO chip)
echo 529 > /sys/class/gpio/export  # GPIO17 for switch
echo 539 > /sys/class/gpio/export  # GPIO27 for LED

# Verify GPIO directories exist
ls /sys/class/gpio/gpio529/
ls /sys/class/gpio/gpio539/
```

## Running the Application

```bash
# Make executable
chmod +x led_control

# Run application
./led_control
```

### Expected Output
```
<================ Control LED App ================>
Usage: 
Press the Hard switch to light up led on gpio pin 27
Switch OFF -> LED OFF
Switch OFF -> LED OFF
Switch ON -> LED ON
Switch ON -> LED ON
```

## GPIO Offset Mapping

This Raspberry Pi configuration uses GPIO chip offset mapping:

| Physical GPIO | Sysfs Path | Offset Calculation |
|---------------|------------|-------------------|
| GPIO17 | `/sys/class/gpio/gpio529/` | 512 + 17 = 529 |
| GPIO27 | `/sys/class/gpio/gpio539/` | 512 + 27 = 539 |

To find your GPIO chip base:
```bash
ls /sys/class/gpio/
# Look for gpiochip* directories
```

## Troubleshooting

### Permission Issues
```bash
# Add user to gpio group
sudo usermod -a -G gpio $USER

# Or run with sudo
sudo ./led_control
```

### GPIO Already Exported
```bash
# Unexport if needed
echo 529 > /sys/class/gpio/unexport
echo 539 > /sys/class/gpio/unexport
```

### File Access Errors
- Check if GPIO pins are properly exported
- Verify file permissions on `/sys/class/gpio/gpio*/` directories
- Ensure correct GPIO offset mapping for your Pi model

### LED Not Working
- Check circuit connections
- Verify LED polarity (longer leg to GPIO, shorter to resistor)
- Test with multimeter if available

## Application Flow

1. **Initialization**: Set GPIO27 as output, GPIO17 as input
2. **Main Loop**: 
   - Read switch state from GPIO17
   - Set LED state on GPIO27 accordingly
   - Print status message
   - Wait 500ms before next iteration

## Cleanup

The application runs indefinitely. To stop:
- Press `Ctrl+C`
- GPIO pins remain exported after exit

To clean up manually:
```bash
echo 529 > /sys/class/gpio/unexport
echo 539 > /sys/class/gpio/unexport
```