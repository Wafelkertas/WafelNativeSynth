# Project: NativeSynth
# Structure: Android Native Project with Java/Kotlin UI and C++ Audio Engine (CMake)

cmake_minimum_required(VERSION 3.22.1)
project(NativeSynth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(native-synth SHARED native_synth.cpp)
find_package(oboe REQUIRED CONFIG)

target_link_libraries(native-synth
        oboe::oboe
        log
)