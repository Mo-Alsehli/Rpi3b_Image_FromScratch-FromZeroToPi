# U-Boot Guide for Raspberry Pi

A practical guide for using U-Boot with Raspberry Pi across three development stages.

## Serial Connection Setup

### Hardware Connection
Connect USB-to-TTL adapter to Raspberry Pi GPIO:

```
USB-TTL Adapter    Raspberry Pi GPIO
GND      ←→        Pin 6  (Ground)
RX       ←→        Pin 8  (GPIO 14 - TX)
TX       ←→        Pin 10 (GPIO 15 - RX)
VCC      ←→        DO NOT CONNECT
```

**Warning**: Never connect VCC - it can damage your Pi.

### Connect to Serial Console
```bash
# Install terminal software
sudo apt install screen

# Connect (adjust /dev/ttyUSB0 to your device)
screen /dev/ttyUSB0 115200

# Alternative with picocom
sudo apt install picocom
picocom -b 115200 /dev/ttyUSB0
```

To exit screen: Press `Ctrl+A` then `K`, then `Y`

---

## Stage 1: U-Boot Only

### What's on SD Card
```
SD Card (FAT32 partition)
├── bootcode.bin
├── start.elf
├── fixup.dat
├── bcm2710-rpi-3-b.dtb
├── u-boot.bin
└── config.txt
```

### Power On Test
1. Insert SD card and power on Pi
2. Watch serial console - you should see:

```
U-Boot 2025.10 (date/time)
DRAM:  948 MiB
RPI 3 Model B
Hit any key to stop autoboot: 3 2 1 0
=>
```

3. Press any key to get U-Boot prompt (`=>`)

### Basic U-Boot Commands
```bash
# List files on SD card
=> fatls mmc 0:1

# Show environment variables  
=> printenv

# Show board info
=> bdinfo

# Show help
=> help

# Test memory
=> mtest 0x1000000 0x2000000 1
```

### Expected Output
```bash
=> fatls mmc 0:1
   262144   bootcode.bin
    52296   start.elf  
     6666   fixup.dat
    25674   bcm2710-rpi-3-b.dtb
   872448   u-boot.bin
       89   config.txt
```

**Success**: You have working U-Boot if you see the prompt and can run commands.

---

## Stage 2: U-Boot + Linux Kernel

### What's Added to SD Card
```
SD Card (FAT32 partition)
├── bootcode.bin
├── start.elf
├── fixup.dat
├── bcm2710-rpi-3-b.dtb
├── u-boot.bin
├── config.txt
└── Image                   # Linux kernel (NEW)
```

### Manual Kernel Boot Test
At U-Boot prompt:

```bash
# Check if kernel exists
=> fatls mmc 0:1
# Should show "Image" file

# Load kernel into memory
=> load mmc 0:1 ${kernel_addr_r} Image

# Load device tree
=> load mmc 0:1 ${fdt_addr_r} bcm2710-rpi-3-b.dtb

# Set basic boot parameters
=> setenv bootargs "console=ttyS0,115200 panic=10"

# Boot kernel
=> booti ${kernel_addr_r} - ${fdt_addr_r}
```

### Expected Result
Kernel will start but panic with "Unable to mount root fs" - this is normal for Stage 2.

You should see:
```
Starting kernel ...
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 6.1.21+
[    0.000000] Machine model: Raspberry Pi 3 Model B
...
[    2.337890] Kernel panic - not syncing: VFS: Unable to mount root fs
```

**Success**: Kernel loads and starts (panic is expected without rootfs).

---

## Stage 3: U-Boot + Kernel + Root Filesystem

### SD Card Structure
```
SD Card
├── Partition 1 (FAT32, ~256MB) - Boot
│   ├── bootcode.bin
│   ├── start.elf
│   ├── fixup.dat
│   ├── bcm2710-rpi-3-b.dtb
│   ├── u-boot.bin
│   ├── config.txt
│   ├── Image
│   └── boot.scr            # Auto-boot script (NEW)
└── Partition 2 (EXT4) - Root filesystem (NEW)
    ├── bin/
    ├── sbin/
    ├── lib/
    └── etc/
```

### Create Auto-Boot Script
Create `boot.cmd`:
```bash
# Auto-boot script
load mmc 0:1 ${kernel_addr_r} Image
load mmc 0:1 ${fdt_addr_r} bcm2710-rpi-3-b.dtb
setenv bootargs "root=/dev/mmcblk0p2 rw rootfstype=ext4 console=ttyS0,115200 init=/sbin/init rootwait"
booti ${kernel_addr_r} - ${fdt_addr_r}
```

Convert to binary format:
```bash
mkimage -C none -A arm64 -T script -d boot.cmd boot.scr
```

### Automatic Boot Test
1. Power on Pi (no interaction needed)
2. U-Boot should auto-boot after 3 seconds
3. System should boot to shell prompt

Expected sequence:
```
U-Boot 2025.10
Hit any key to stop autoboot: 3 2 1 0
## Executing script at 00200000
Loading Image...
Loading device tree...
Starting kernel ...
[kernel messages...]
Starting system...

Please press Enter to activate this console.
/ #
```

4. Press Enter to get shell prompt (`/ #`)

### Test System Commands
```bash
# Check system info
/ # uname -a

# List files
/ # ls /bin

# Check processes
/ # ps

# Check mounted filesystems  
/ # mount

# Test file operations
/ # echo "test" > /tmp/hello.txt
/ # cat /tmp/hello.txt

# Reboot
/ # reboot
```

**Success**: You get a working shell with basic Linux commands.

---

## Common U-Boot Issues

### Problem: No U-Boot prompt
**Symptoms**: No output on serial console
**Solutions**:
- Check serial connections
- Verify baud rate is 115200
- Ensure `enable_uart=1` in config.txt
- Try different USB-TTL adapter

### Problem: "mmc not found"
**Symptoms**: U-Boot can't read SD card
**Solutions**:
- Try different SD card
- Reformat SD card as FAT32
- Check SD card is properly inserted

### Problem: Kernel won't load
**Symptoms**: "Bad Linux ARM64 Image magic!"
**Solutions**:
- Verify Image file is correct ARM64 kernel
- Check file isn't corrupted
- Rebuild kernel with correct settings

### Problem: Kernel panic in Stage 3
**Symptoms**: "No working init found"
**Solutions**:
- Check rootfs partition exists
- Verify `/sbin/init` exists and is executable
- Check bootargs root device is correct

---

## Quick Commands Reference

### U-Boot Essential Commands
```bash
fatls mmc 0:1              # List boot partition files
load mmc 0:1 <addr> <file> # Load file to memory address
printenv                   # Show all variables
setenv <var> <value>       # Set variable
saveenv                    # Save variables permanently
booti <addr> - <fdt_addr>  # Boot ARM64 kernel
reset                      # Restart system
```

### Stop Auto-Boot
- Press any key during 3-second countdown
- Or hold key while powering on

### Resume Auto-Boot
```bash
=> boot
```

This guide covers the essential U-Boot operations for each stage. The serial connection lets you interact with U-Boot and see the complete boot process, making debugging much easier.