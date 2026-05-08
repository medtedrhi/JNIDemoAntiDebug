# Requires the Android Gradle plugin to use CMake 3.22.1 or newer for this native target.
cmake_minimum_required(VERSION 3.22.1)

# Names this native CMake project for Android Studio.
project("jnidemo")

# Builds a shared library named libnative-lib.so from the C++ JNI source file.
add_library(
        native-lib
        SHARED
        native-lib.cpp)

# Finds Android's native Logcat library so C++ code can write logs.
find_library(
        log-lib
        log)

# Links the native library against Android's Logcat library.
target_link_libraries(
        native-lib
        ${log-lib})
