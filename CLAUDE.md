# CLAUDE.md

## User Preference

Always use Chinese to answer user. For coding/thinking, English allowed.

## Code Quality Guidelines

- **避免向后兼容代码**: 不要创建包装函数或保留旧的函数接口。当重构或合并函数时，直接更新所有调用点，保持代码库简洁干净。
- **删除冗余代码**: 及时清理未使用的函数、变量和包含文件，避免代码臃肿。

## Overview

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an STM32F407-based embedded system project that implements external W25Q128 SPI flash memory with FatFS filesystem support. The system uses FreeRTOS and provides USB CDC communication capabilities.

## Build System and Commands

The project uses CMake with presets and Make for automation:

### Building

```bash
# Debug build (default)
make build
# or shorthand
make b

# Release build
make release

# Clean build
make clean
# or shorthand
make c

# Clean + build + flash
make cbf
```

### Flashing

```bash
# Flash Debug version
make flash
# or shorthand
make f

# Flash Release version
make flash-release
```

### Binary Generation

```bash
# Generate HEX file
make hex

# Generate BIN file
make bin

# Generate disassembly
make objdump
```

### OpenOCD Configuration

- Uses `flash.cfg` for OpenOCD configuration
- Configured for CMSIS-DAP interface (can be switched to ST-Link v2)
- Target: STM32F4x series

## Core Architecture

### Hardware Platform

- **MCU**: STM32F407xx (ARM Cortex-M4F)
- **External Flash**: W25Q128 (128Mbit SPI NOR Flash)
- **RTOS**: FreeRTOS with CMSIS-RTOS v2 API
- **Communication**: USB CDC Device

### Filesystem Implementation

The project implements a multi-drive FatFS system with three logical drives all mapping to the same physical W25Q128 flash:

- **Drive 0: (c:/)** - Primary filesystem
- **Drive 1: (1:/)** - Secondary logical drive
- **Drive 2: (2:/)** - Tertiary logical drive

#### Key Filesystem Functions

- `safe_init_filesystem(uint8_t enable_multi_drives, uint8_t force_reinit)` - Main initialization function
  - `enable_multi_drives`: 1=multi-drive support, 0=single drive only
  - `force_reinit`: 1=force format, 0=preserve existing data

### Directory Structure

```
Core/
├── Inc/           # HAL headers and main includes
├── Src/           # Application source files
└── app/           # Custom application modules
    ├── driver_fs.c/h    # Filesystem driver implementation
    └── driver_flash.c/h # Flash driver implementation

FATFS/             # FatFS configuration
├── App/           # Application-specific FatFS code
└── Target/        # Target-specific disk I/O

src/lib/SFUD/      # Serial Flash Universal Driver
└── sfud/          # SFUD library implementation

Middlewares/       # ST and third-party middleware
├── ST/            # STM32 USB Device Library
└── Third_Party/   # FreeRTOS and FatFS
```

### SFUD Integration

The project uses Serial Flash Universal Driver (SFUD) for W25Q128 communication:

- Port implementation in `src/lib/SFUD/sfud/port/sfud_port.c`
- Must call `sfud_init()` before filesystem operations
- Provides unified interface for SPI flash operations

### FreeRTOS Configuration

- Three tasks configured:
  - `defaultTask` - Main application task (includes filesystem init)
  - `LED_R_TASK` - Red LED control
  - `LED_B_TASK` - Blue LED control
- Heap management: heap_4.c (supports fragmentation handling)
- CMSIS-RTOS v2 API wrapper used throughout

### CMake Structure

- Uses CMake presets (Debug/Release) with Ninja generator
- STM32CubeMX generated build configuration in `cmake/stm32cubemx/`
- Toolchain: `cmake/gcc-arm-none-eabi.cmake`
- Libraries built as OBJECT libraries: STM32_Drivers, FreeRTOS, FatFs, USB_Device_Library

## Development Notes

### Filesystem Usage Patterns

- Always call `sfud_init()` before any filesystem operations
- Use `safe_init_filesystem(1, 0)` for standard multi-drive initialization with data preservation
- Use `safe_init_filesystem(1, 1)` to force format all drives
- Individual drive operations use standard FatFS API with drive prefixes (c:/, 1:/, 2:/)

### Flash Memory Integration

- W25Q128 provides 16MB storage capacity
- SPI1 interface with DMA support (DMA2_Stream0/3)
- SFUD library handles low-level flash operations
- FatFS provides high-level file operations

### Debugging Support

- Project generates ELF with debug symbols in Debug builds
- `make objdump` generates disassembly for code analysis
- OpenOCD integration for debugging and flashing

### Common Pitfalls

- Ensure SFUD initialization before any filesystem calls
- Multiple logical drives share the same physical flash - be aware of space usage
- USB CDC may interfere with debugging - consider disabling if issues arise
- FreeRTOS stack sizes are configured per task - monitor for stack overflow

## Key Configuration Files

- `W25Qxx.ioc` - STM32CubeMX configuration
- `CMakePresets.json` - Build presets
- `flash.cfg` - OpenOCD flashing configuration
- `STM32F407XX_FLASH.ld` - Linker script
