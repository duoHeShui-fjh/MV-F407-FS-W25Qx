cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME W25Qxx)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user sources here
    Core/app/driver_fs.c
    Core/app/app.c

    # hal
    src/hal/hal_delay.c
    src/hal/hal_uart.c
    src/hal/hal_gpio.c

    # drivers
    src/drivers/comm/driver_log.c
    src/drivers/comm/driver_cmd.c
    src/drivers/json/driver_json.c

    # lib
    src/lib/cJSON/cJSON.c
    src/lib/SFUD/sfud/src/sfud.c
    src/lib/SFUD/sfud/src/sfud_sfdp.c
    src/lib/SFUD/sfud/port/sfud_port.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user defined include paths
    src/hal

    src/drivers
    src/drivers/comm
    src/drivers/device
    src/drivers/json

    src/lib/cJSON
    src/lib/SFUD/sfud/port
    src/lib/SFUD/sfud/src
    src/lib/SFUD/sfud/inc

    Core/app
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
