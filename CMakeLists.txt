cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS components)

# Suppress harmless warnings in transitive dependency (esp-modbus)
add_compile_options(-Wno-error=stringop-truncation)

project(audio_mixer_p4)
