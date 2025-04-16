rem @echo off
rmdir /S /Q Win
mkdir Win
cd Win
cmake ../ -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake  -G "Visual Studio 17 2022"
cd ..
