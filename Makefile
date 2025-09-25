# CoreXY STM32F103 Project Makefile
# 项目配置 (自动检测项目名)
BUILD_DIR_DEBUG = build/Debug
BUILD_DIR_RELEASE = build/Release
BUILD_TYPE = Debug

ELF_FILE = $(shell find build/Debug -name "*.elf" -type f 2>/dev/null | head -1)

# 自动检测ELF文件名的函数
get_elf_file = $(shell find $(1) -name "*.elf" -type f 2>/dev/null | head -1)
get_project_name = $(shell basename $(call get_elf_file,$(1)) .elf 2>/dev/null)

# CMake 配置
CMAKE_PRESET_DEBUG = Debug
CMAKE_PRESET_RELEASE = Release

# OpenOCD 配置
OPENOCD_CFG = flash.cfg

# 自动检测芯片型号 (从CMake配置文件中获取)
MCU_TYPE = $(shell grep -o 'STM32F[0-9][0-9][0-9]' cmake/stm32cubemx/CMakeLists.txt 2>/dev/null | head -1)


# 默认目标
.PHONY: default clean build release flash flash-release c b f cb bf cbf objdump bin hex all clangd

default: build
c: clean
b: build
f: flash
cb: clean build
bf: build flash
cbf: clean build flash
all:b f hex bin

# clean: 清理构建文件
clean:
	@echo "清理构建文件..."
	@rm -rf build/
	@echo "清理完成"

# build: 构建Debug版本 (包含初始化检查)
build:
	@echo "配置 CMake 项目 (Debug)..."
	@cmake --preset $(CMAKE_PRESET_DEBUG)
	@echo "构建Debug版本..."
	@cmake --build $(BUILD_DIR_DEBUG)
	@echo "Debug构建完成: $(ELF_FILE)"


# flash: 烧录固件到MCU (默认使用Debug版本)
flash:
	@echo "烧录固件到 $(MCU_TYPE)..."
	@echo "找到ELF文件: $(ELF_FILE)"
	openocd -f flash.cfg \
		-c "program $(ELF_FILE) verify reset exit"
	@echo "烧录完成！"

# flash-release: 烧录Release版本固件
flash-release:
	@echo "烧录Release固件到 $(MCU_TYPE)..."
	@ELF_FILE=$$(find $(BUILD_DIR_RELEASE) -name "*.elf" -type f 2>/dev/null | head -1); \
	if [ -z "$$ELF_FILE" ]; then \
		echo "错误: 在 $(BUILD_DIR_RELEASE) 中找不到 .elf 文件，请先运行 'make release'"; \
		exit 1; \
	fi; \
	echo "找到ELF文件: $$ELF_FILE"; \
	openocd -f $(OPENOCD_CFG) -c "set ELF_FILE $$ELF_FILE"
	@echo "Release版本烧录完成"


# objdump: 反汇编固件 (Debug版本)
objdump:
	@ELF_FILE=$$(find $(BUILD_DIR_DEBUG) -name "*.elf" -type f 2>/dev/null | head -1); \
	if [ -z "$$ELF_FILE" ]; then \
		echo "错误: 在 $(BUILD_DIR_DEBUG) 中找不到 .elf 文件，请先运行 'make build'"; \
		exit 1; \
	fi; \
	DIS_FILE="$${ELF_FILE%.elf}.dis"; \
	echo "生成反汇编文件: $$DIS_FILE"; \
	arm-none-eabi-objdump -d "$$ELF_FILE" > "$$DIS_FILE"; \
	echo "反汇编文件已生成: $$DIS_FILE"

# bin: 生成BIN文件 (从Debug ELF文件转换)
bin:
	@ELF_FILE=$$(find $(BUILD_DIR_DEBUG) -name "*.elf" -type f 2>/dev/null | head -1); \
	if [ -z "$$ELF_FILE" ]; then \
		echo "错误: 在 $(BUILD_DIR_DEBUG) 中找不到 .elf 文件，请先运行 'make build'"; \
		exit 1; \
	fi; \
	BIN_FILE="$${ELF_FILE%.elf}.bin"; \
	echo "生成BIN文件: $$BIN_FILE"; \
	arm-none-eabi-objcopy -O binary "$$ELF_FILE" "$$BIN_FILE"; \
	echo "BIN文件已生成: $$BIN_FILE"; \
	ls -la "$$BIN_FILE"

# hex: 生成HEX文件 (从Debug ELF文件转换)
hex:
	@ELF_FILE=$$(find $(BUILD_DIR_DEBUG) -name "*.elf" -type f 2>/dev/null | head -1); \
	if [ -z "$$ELF_FILE" ]; then \
		echo "错误: 在 $(BUILD_DIR_DEBUG) 中找不到 .elf 文件，请先运行 'make build'"; \
		exit 1; \
	fi; \
	HEX_FILE="$${ELF_FILE%.elf}.hex"; \
	echo "生成HEX文件: $$HEX_FILE"; \
	arm-none-eabi-objcopy -O ihex "$$ELF_FILE" "$$HEX_FILE"; \
	echo "HEX文件已生成: $$HEX_FILE"; \
	ls -la "$$HEX_FILE"

# release: 构建Release版本
release:
	@echo "配置 CMake 项目 (Release)..."
	@cmake --preset $(CMAKE_PRESET_RELEASE)
	@echo "构建Release版本..."
	@cmake --build $(BUILD_DIR_RELEASE)
	@ELF_FILE=$$(find $(BUILD_DIR_RELEASE) -name "*.elf" -type f 2>/dev/null | head -1); \
	if [ -n "$$ELF_FILE" ]; then \
		echo "Release构建完成: $$ELF_FILE"; \
	else \
		echo "Release构建完成"; \
	fi


clangd: 
	@rm -rf compile_commands.json
	@cp $(BUILD_DIR_DEBUG)/compile_commands.json .