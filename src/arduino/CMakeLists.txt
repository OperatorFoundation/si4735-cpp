cmake_minimum_required(VERSION 3.31)
project(si4735-arduino)

include(${CMAKE_SOURCE_DIR}/lib/cmake-helpers/arduino-sdk.cmake)

set(CMAKE_CXX_STANDARD 17)
set(SI4735_LIB_DIR ${CMAKE_SOURCE_DIR}/src/cpp)
set(SI4735_LIB_SOURCE_DIR ${SI4735_LIB_DIR}/src)
set(SI4735_ARDUINO_DIR ${CMAKE_SOURCE_DIR}/src/arduino)
set(SI4735_ARDUINO_SOURCE_DIR ${SI4735_ARDUINO_DIR}/src)
set(SI4735_ARDUINO_EXAMPLES_DIR ${SI4735_ARDUINO_DIR}/examples)
set(SI4735_ARDUINO_EXAMPLE_DIR ${SI4735_ARDUINO_EXAMPLES_DIR}/si4735_example)

# Define your sketch file
set(SKETCH_FILE ${SI4735_ARDUINO_EXAMPLE_DIR}/si4735_example.ino)

set(BOARD "rp2040:rp2040:adafruit_feather_rp2350_hstx")
set(PORT "/dev/cu.usbmodem1101")

set(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/arduino/rp2350/si4735-i2c)
file(GLOB_RECURSE SI4735_FILES ${SI4735_LIB_SOURCE_DIR}/*.cpp ${SI4735_ARDUINO_SOURCE_DIR}/*.cpp ${SI4735_ARDUINO_SOURCE_DIR}/*.ino)

add_executable(si4735-arduino ${SI4735_FILES}
  src/si4735-arduino.h
  src/SI4735Arduino.cpp
  src/SI4735Arduino.h)
add_dependencies(si4735-arduino build_si4735_arduino_example)
target_link_libraries(si4735-arduino PRIVATE si4735-cpp)
target_link_arduino_sdk(si4735-arduino rp2040)

add_custom_target(build_si4735_arduino_example ALL
        COMMAND arduino-cli compile
            --fqbn ${BOARD}
            --output-dir ${OUTPUT_DIR}
            --build-property "compiler.cpp.extra_flags=-std=c++17 -D_BSD_SOURCE -D_DEFAULT_SOURCE"
            --library ${SI4735_LIB_DIR}
            --library ${SI4735_ARDUINO_DIR}
            --library ${FLUX_CPP}
            --library ${FLUX_ARDUINO}
  ${SKETCH_FILE}
        WORKING_DIRECTORY ${SI4735_ARDUINO_SOURCE_DIR}
        COMMENT "Building si4735-arduino with arduino-cli"
)

add_custom_target(upload_si4735_arduino_example ALL
        COMMAND arduino-cli upload
        -p ${PORT}
        --fqbn ${BOARD}
        ${SI4735_ARDUINO_PROTOCOL_SERVER_DIR}
        COMMENT "Uploading si4735-arduino-protocol-server with arduino-cli"
)
add_dependencies(upload_si4735_arduino_example build_si4735_arduino_example)