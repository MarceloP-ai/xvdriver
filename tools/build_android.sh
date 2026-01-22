#!/bin/bash

NDK_PATH=$ANDROID_NDK_HOME
SYSROOT=$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/sysroot

cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-31 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build --target xvdriver