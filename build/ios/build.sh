#!/bin/sh
set -e

mkdir -p os
cd os
mkdir -p debug
cd debug
cmake -G "Xcode" -DUNIX=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET=10.0 -DENABLE_BITCODE=FALSE -DENABLE_ARC=FALSE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CONFIGURATION_TYPES=Debug ../../../../srcs
cmake --build .
cd ..
mkdir -p release
cd release
cmake -G "Xcode" -DUNIX=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET=10.0 -DENABLE_BITCODE=FALSE -DENABLE_ARC=FALSE -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release ../../../../srcs
cmake --build .
cd ..
cd ..

mkdir -p simulator64
cd simulator64
mkdir -p debug
cd debug
cmake -G "Xcode" -DUNIX=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR64 -DIOS_DEPLOYMENT_TARGET=10.0 -DENABLE_BITCODE=FALSE -DENABLE_ARC=FALSE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CONFIGURATION_TYPES=Debug ../../../../srcs
cmake --build .
cd ..
mkdir -p release
cd release
cmake -G "Xcode" -DUNIX=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR64 -DIOS_DEPLOYMENT_TARGET=10.0 -DENABLE_BITCODE=FALSE -DENABLE_ARC=FALSE -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release ../../../../srcs
cmake --build .
cd ..
cd ..
