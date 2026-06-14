cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ESP32-P4 Audio Mixer - IDF project
# This file exists so GitHub Actions can compile with ESP-IDF

set(EXTRA_COMPONENT_DIRS components)

project(audio_mixer_p4)
