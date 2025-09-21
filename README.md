# 🥧 Raspberry Pi Boot Process - Complete Build Guide

A comprehensive guide for building a complete Raspberry Pi boot system from scratch, covering three progressive stages: U-Boot bootloader, Linux kernel integration, and BusyBox root filesystem.

## 📋 Project Overview

This repository contains everything needed to build a custom Raspberry Pi system from source, organized in three progressive stages:

- **Stage 1**: U-Boot bootloader setup
- **Stage 2**: U-Boot with Raspberry Pi Linux kernel
- **Stage 3**: Complete system with BusyBox root filesystem
- **Note**: A full guide to boot on raspberry @ each step is explained @ u-boot-guide-raspi.md

## 🗂️ Repository Structure

```
rpi-boot-system/
├── 01-cross-ng-build/          # Cross-compilation toolchain
│   ├── aarch64-rpi3-linux-gnu/ # Built toolchain
│   └── sysroot/                # Target system libraries
├── 02-u-boot/                  # U-Boot bootloader
│   └── u-boot.bin              # Compiled bootloader
├── 03-rpi-firmware/            # Raspberry Pi firmware files
│   ├── bootcode.bin
│   ├── start.elf
│   ├── fixup.dat
│   └── bcm2710-rpi-3-b.dtb
├── 04-busybox-rootfs/          # BusyBox root filesystem
│   ├── _install/               # Built BusyBox system
│   └── lib/                    # Required libraries
├── 05-rpi-linux-build/         # Linux kernel build
│   └── Image                   # Compiled kernel
├── 06-final-sdcard/            # Final SD card images
│   ├── bootfs/                 # Boot partition
│   └── rootfs/                 # Root filesystem
├── README.md
└── u-boot-guide-raspi.md
```

---

## 🔧 Prerequisites

### System Requirements
- Ubuntu 20.04+ or similar Linux distribution
- 8GB+ RAM recommended
- 20GB+ free disk space
- SD card (16GB+ recommended)

### Required Packages
```bash
sudo apt update
sudo apt install -y \
    build-essential git wget curl \
    libncurses5-dev libncursesw5-dev \
    libtirpc-dev bison flex \
    bc device-tree-compiler \
    u-boot-tools dosfstools
```

---

## 📖 Stage 1: U-Boot Bootloader

### Goal
Set up U-Boot as a flexible bootloader for Raspberry Pi 3, providing a command-line interface for loading and executing various images.

### 1.1 Build Cross-Compilation Toolchain

Using crosstool-ng to build a custom aarch64 toolchain:

```bash
# Clone and build crosstool-ng
git clone https://github.com/crosstool-ng/crosstool-ng.git
cd crosstool-ng
./configure --prefix=/opt/crosstool-ng
make && sudo make install

# Create toolchain directory
mkdir -p 01-cross-ng-build
cd 01-cross-ng-build

# Configure for Raspberry Pi 3 (aarch64)
/opt/crosstool-ng/bin/ct-ng aarch64-rpi3-linux-gnu
/opt/crosstool-ng/bin/ct-ng menuconfig  # Optional: customize settings

# Build toolchain (takes 30-60 minutes)
/opt/crosstool-ng/bin/ct-ng build

# Export toolchain path
export PATH="$PWD/x-tools/aarch64-rpi3-linux-gnu/bin:$PATH"
export CROSS_COMPILE=aarch64-rpi3-linux-gnu-
```

### 1.2 Build U-Boot

```bash
# Clone U-Boot repository
git clone https://github.com/u-boot/u-boot.git 02-u-boot
cd 02-u-boot

# Configure for Raspberry Pi 3
export ARCH=arm64
export CROSS_COMPILE=aarch64-rpi3-linux-gnu-
make rpi_3_defconfig

# Build U-Boot
make -j$(nproc)

# Output: u-boot.bin
ls -la u-boot.bin
```

### 1.3 Prepare Raspberry Pi Firmware

```bash
# Create firmware directory
mkdir -p 03-rpi-firmware
cd 03-rpi-firmware

# Download official Raspberry Pi firmware
wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat

# Download device tree blob
wget https://github.com/raspberrypi/firmware/raw/master/boot/bcm2710-rpi-3-b.dtb
```

### 1.4 SD Card Setup (Stage 1)

```bash
# Format SD card (assuming /dev/sdb - CHECK YOUR DEVICE!)
sudo fdisk /dev/sdb
# Create FAT32 partition (256MB)

# Format and mount
sudo mkfs.fat -F 32 /dev/sdb1
sudo mkdir -p /mnt/rpi-boot
sudo mount /dev/sdb1 /mnt/rpi-boot

# Copy files to boot partition
sudo cp 03-rpi-firmware/* /mnt/rpi-boot/
sudo cp 02-u-boot/u-boot.bin /mnt/rpi-boot/

# Create config.txt
sudo tee /mnt/rpi-boot/config.txt << 'EOF'
kernel=u-boot.bin
arm_64bit=1
enable_uart=1
core_freq=250
EOF

sudo umount /mnt/rpi-boot
```

### Stage 1 SD Card Structure
```
SD Card (FAT32)
├── bootcode.bin        # First stage bootloader
├── start.elf          # GPU firmware
├── fixup.dat          # GPU configuration
├── bcm2710-rpi-3-b.dtb # Device tree blob
├── u-boot.bin         # Our bootloader
└── config.txt         # Configuration file
```

**Test Result**: Boot Raspberry Pi - you should see U-Boot prompt (`=>`) on serial console (115200 baud).

---

## 🐧 Stage 2: U-Boot with Linux Kernel

### Goal
Add a custom-built Linux kernel that can be loaded and executed by U-Boot.

### 2.1 Build Linux Kernel

```bash
# Clone Raspberry Pi Linux repository
git clone https://github.com/raspberrypi/linux.git 05-rpi-linux-build
cd 05-rpi-linux-build

# Set build environment
export ARCH=arm64
export CROSS_COMPILE=aarch64-rpi3-linux-gnu-

# Configure kernel for Raspberry Pi 3
make bcmrpi3_defconfig

# Optional: customize kernel
make menuconfig

# Build kernel and modules
make -j$(nproc)
make modules

# Install modules to temporary directory
make INSTALL_MOD_PATH=../04-busybox-rootfs modules_install
```

### 2.2 Update SD Card (Stage 2)

```bash
# Mount boot partition
sudo mount /dev/sdb1 /mnt/rpi-boot

# Copy kernel image
sudo cp 05-rpi-linux-build/arch/arm64/boot/Image /mnt/rpi-boot/

# Copy device tree (if building custom DTB)
sudo cp 05-rpi-linux-build/arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb /mnt/rpi-boot/

sudo umount /mnt/rpi-boot
```

### 2.3 U-Boot Commands for Kernel Loading

Boot the Pi and at U-Boot prompt, test manual kernel loading:

```bash
# Load kernel into memory
load mmc 0:1 ${kernel_addr_r} Image

# Load device tree
load mmc 0:1 ${fdt_addr_r} bcm2710-rpi-3-b.dtb

# Set kernel command line (minimal - will fail without rootfs)
setenv bootargs "console=ttyS0,115200 panic=10"

# Boot kernel
booti ${kernel_addr_r} - ${fdt_addr_r}
```

### Stage 2 SD Card Structure
```
SD Card (FAT32)
├── bootcode.bin
├── start.elf
├── fixup.dat
├── bcm2710-rpi-3-b.dtb
├── u-boot.bin
├── config.txt
└── Image               # Linux kernel (NEW)
```

**Test Result**: Kernel should load but will panic due to missing root filesystem.

---

## 📦 Stage 3: Complete System with BusyBox

### Goal
Create a minimal but functional Linux system with BusyBox providing essential utilities.

### 3.1 Build BusyBox Root Filesystem

```bash
# Clone BusyBox
git clone -b 1_36_stable https://github.com/mirror/busybox.git --depth=1
cd busybox

# Configure cross-compilation
export CROSS_COMPILE=aarch64-rpi3-linux-gnu-

# Use default configuration
make defconfig

# Optional: customize BusyBox features
make menuconfig

# Build BusyBox
make -j$(nproc)

# Install to _install directory
make install

# Create root filesystem structure
mkdir -p ../04-busybox-rootfs
rsync -av _install/ ../04-busybox-rootfs/
```

### 3.2 Add Required Libraries

```bash
cd 04-busybox-rootfs

# Create library directories
mkdir -p lib lib64

# Copy libraries from toolchain sysroot
SYSROOT="$(aarch64-rpi3-linux-gnu-gcc -print-sysroot)"
rsync -av ${SYSROOT}/lib/ lib/
rsync -av ${SYSROOT}/lib64/ lib64/

# Create essential directories
mkdir -p {dev,proc,sys,tmp,var,etc,root}
chmod 1777 tmp
```

### 3.3 Create Init Configuration

```bash
# Create inittab for BusyBox init
cat > etc/inittab << 'EOF'
# System initialization
null::sysinit:/bin/mount -t proc proc /proc
null::sysinit:/bin/mount -t sysfs sysfs /sys  
null::sysinit:/bin/mount -t devtmpfs devtmpfs /dev

# Start shell on console
console::askfirst:/bin/ash

# Restart init on Ctrl+Alt+Del
null::ctrlaltdel:/sbin/reboot

# Shutdown
null::shutdown:/bin/umount -a -r
EOF

# Create a simple rcS startup script
mkdir -p etc/init.d
cat > etc/init.d/rcS << 'EOF'
#!/bin/ash
echo "Starting system..."
/bin/mount -o remount,rw /
/bin/hostname -F /etc/hostname 2>/dev/null || /bin/hostname "rpi-busybox"
echo "System startup complete."
EOF
chmod +x etc/init.d/rcS

# Create hostname file
echo "rpi-busybox" > etc/hostname
```

### 3.4 Setup Complete SD Card

```bash
# Create second partition for rootfs (assuming /dev/sdb2)
sudo fdisk /dev/sdb
# Add second partition using remaining space, type Linux (83)

# Format root filesystem
sudo mkfs.ext4 /dev/sdb2

# Mount both partitions
sudo mkdir -p /mnt/{rpi-boot,rpi-root}
sudo mount /dev/sdb1 /mnt/rpi-boot
sudo mount /dev/sdb2 /mnt/rpi-root

# Copy rootfs
sudo rsync -av 04-busybox-rootfs/ /mnt/rpi-root/

# Ensure proper permissions
sudo chown -R root:root /mnt/rpi-root
sudo chmod 755 /mnt/rpi-root
sudo chmod 4755 /mnt/rpi-root/bin/busybox

# Update boot configuration for automatic boot
sudo tee /mnt/rpi-boot/boot.cmd << 'EOF'
# Automatic boot script
load mmc 0:1 ${kernel_addr_r} Image
load mmc 0:1 ${fdt_addr_r} bcm2710-rpi-3-b.dtb
setenv bootargs "root=/dev/mmcblk0p2 rw rootfstype=ext4 console=ttyS0,115200 init=/sbin/init rootwait"
booti ${kernel_addr_r} - ${fdt_addr_r}
EOF

# Generate boot.scr
mkimage -C none -A arm64 -T script -d /mnt/rpi-boot/boot.cmd /mnt/rpi-boot/boot.scr

# Unmount
sudo umount /mnt/rpi-boot /mnt/rpi-root
```

### Final SD Card Structure

```
SD Card
├── Partition 1 (FAT32, ~256MB) - Boot Filesystem
│   ├── bootcode.bin
│   ├── start.elf  
│   ├── fixup.dat
│   ├── bcm2710-rpi-3-b.dtb
│   ├── u-boot.bin
│   ├── config.txt
│   ├── Image
│   ├── boot.cmd
│   └── boot.scr            # Auto-boot script
└── Partition 2 (EXT4, rest) - Root Filesystem
    ├── bin/                # BusyBox utilities
    ├── sbin/               # System binaries
    ├── lib/                # Shared libraries
    ├── lib64/              # 64-bit libraries
    ├── etc/
    │   ├── inittab         # Init configuration
    │   ├── hostname
    │   └── init.d/rcS      # Startup script
    ├── dev/                # Device nodes (mounted by kernel)
    ├── proc/               # Process filesystem (virtual)
    ├── sys/                # System filesystem (virtual)  
    ├── tmp/                # Temporary files
    ├── var/                # Variable data
    └── root/               # Root user home
```

---

## 🚀 Testing the Complete System

1. **Insert SD card** into Raspberry Pi 3
2. **Connect serial cable** (115200 baud, 8N1)
3. **Power on** the device

Expected boot sequence:
```
1. BootROM → bootcode.bin
2. bootcode.bin → start.elf  
3. start.elf → u-boot.bin
4. U-Boot executes boot.scr
5. U-Boot loads kernel and DTB
6. Linux kernel boots
7. BusyBox init starts
8. Shell prompt appears
```

## 🛠️ Troubleshooting

### Common Issues

**U-Boot doesn't start**
- Check `config.txt` syntax
- Verify `u-boot.bin` is not corrupted
- Ensure SD card is properly formatted (FAT32)

**Kernel panic - no init found**
- Check `bootargs` in boot script
- Verify root filesystem partition is correct
- Ensure `/sbin/init` exists and is executable

**BusyBox commands not found**
- Verify libraries are copied correctly
- Check architecture matches (aarch64)
- Use `ldd` on host to check dependencies

### Debug Commands

**In U-Boot:**
```bash
printenv                    # Show all variables
fatls mmc 0:1              # List boot partition files
md ${kernel_addr_r}        # Check loaded kernel
```

**In Linux:**
```bash
ps                         # Show running processes  
mount                      # Show mounted filesystems
cat /proc/version          # Kernel version
cat /proc/cpuinfo         # CPU information
```

---

## 📚 Additional Resources

- [Raspberry Pi Documentation](https://www.raspberrypi.com/documentation/)
- [U-Boot Documentation](https://u-boot.readthedocs.io/)
- [BusyBox Documentation](https://busybox.net/documentation.html)
- [Linux Kernel Documentation](https://kernel.org/doc/)

---

## 🤝 Contributing

Contributions are welcome! Please read the contributing guidelines and submit pull requests for any improvements.

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.