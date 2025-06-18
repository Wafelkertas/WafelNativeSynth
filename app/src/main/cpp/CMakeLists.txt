# Project: NativeSynth
# Structure: Android Native Project with Java/Kotlin UI and C++ Audio Engine (CMake)

cmake_minimum_required(VERSION 3.10.2)
project(NativeSynth)

add_library(
        native-synth
        SHARED
        native_synth.cpp
)

find_library(
        log-lib
        log
)

target_link_libraries(
        native-synth
        android
        log
        OpenSLES
)