#!/bin/bash -e

# to be called from the pub/ directory (here)
usage="usage: toolchain-upload.sh OS(linux/windows/macos) ARCH(i386/amd64/arm32/arm64)"

FRAISE_PATH=$PWD/..
VERSION=`git describe --abbrev=0`

oses="linux macos windows"
arches="i386 amd64 arm32 arm64"

os=$1
arch=$2

declare -A deken_arch
deken_arch[linux]=Linux
deken_arch[macos]=Darwin
deken_arch[windows]=Windows

declare -A deken_arch
deken_arch[i386]=i386
deken_arch[amd64]=amd64
deken_arch[arm32]=arm
deken_arch[arm64]=arm64

in_array() {
    local key=$1
    shift
    local a="$@"
    if [[ ${a[*]} =~ (^|[[:space:]])$key($|[[:space:]]) ]] ; then
        return 0
    else
        return 1
    fi
}

if ! $(in_array $os $oses) ; then
    echo error: unknown os $os
    echo $usage
    exit
    fi

if ! $(in_array $arch $arches) ; then
    echo error: unknown arch $arch
    echo $usage
    exit
    fi

extract="tar -xzf"

case $os-$arch in
    linux-amd64)
        binpath=linux64 
        cmake_url=https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
        gcc_url=https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
        ;;
    linux-arm32)
        binpath=linuxarm
        ;;
    windows-amd64)
        binpath=windows
        cmake_url=https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-windows-x86_64.zip
        gcc_url=https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip
        extract="unzip"
        ;;
    macos-amd64)
        binpath=macos
        cmake_url=https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-macos-universal.tar.gz
        gcc_url=https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-darwin-x86_64-arm-none-eabi.tar.xz
        ;;
    macos-arm64)
        binpath=macos
        cmake_url=https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-macos-universal.tar.gz
        gcc_url=https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-darwin-arm64-arm-none-eabi.tar.xz
        ;;
    *) echo "os-arch not supported (yet)"
    return ;;
    esac

mkdir -p toolchain-build/toolchain/bin
cd toolchain-build

# ----------------- copy bins
cp $FRAISE_PATH/pied/bin/$binpath/* toolchain/bin

# ----------------- clone pico-sdk

if ! [ -e toolchain/pico-sdk ] ; then
    git clone --recursive https://github.com/raspberrypi/pico-sdk.git toolchain/pico-sdk
    fi

# remove unused stuff in sdk-pico

cd toolchain/pico-sdk
rm -rf .git/ docs/* test/ lib/mbedtls/tests
touch docs/CMakeLists.txt
cd lib/tinyusb/hw
    mv mcu/raspberry_pi .
    mv bsp/rp2040 bsp/family_support.cmake .
    rm -rf mcu/*
    rm -rf bsp/*
    mv raspberry_pi mcu/
    mv rp2040 family_support.cmake bsp/
    cd -
cd lib/tinyusb/src/
    mv portable/raspberrypi .
    rm -rf portable/*
    mv raspberrypi portable/
    cd -
rm -rf lib/tinyusb/{lib,test,examples}

rm -rf lib/btstack/{port, test,example,doc,3rd-party/lwip}
cd ../..

# ----------------- get cmake

cmake_file=$(basename $cmake_url)
cmake_dir="${cmake_file%.*}" # remove ".gz" or ".zip"
if [ os != windows ] ; then cmake_dir="${cmake_dir%.*}" ; fi # remove ".tar"

if ! [ -e toolchain/cmake ] ; then
    if ! [ -e $cmake_file ] ; then
        wget $cmake_url
    fi
    $extract $cmake_file
    mv $cmake_dir toolchain/cmake
fi

# remove unused stuff in cmake

rm -rf toolchain/cmake/bin/{cmake-gui,ccmake,ctest,cpack}
rm -rf toolchain/cmake/{doc,man}

# ----------------- get gcc

gcc_file=$(basename $gcc_url)
gcc_dir="${gcc_file%.*}" # remove ".xz"
gcc_dir="${gcc_dir%.*}" # remove ".tar"

if ! [ -e toolchain/gcc ] ; then
    if ! [ -e $gcc_file ] ; then
        wget $gcc_url
    fi
    mkdir -p toolchain/gcc
    tar -xJf $gcc_file -C toolchain/gcc --strip-components=1
fi

# remove unused stuff in gcc

cd toolchain/gcc
cd arm-none-eabi/lib
    mv thumb/nofp .
    rm -rf thumb/*
    mv nofp thumb/
    cd -
cd lib/gcc/arm-none-eabi/13.2.1/
    mv thumb/nofp .
    rm -rf thumb/*
    mv nofp thumb/
    cd -
cd arm-none-eabi/include/c++/13.2.1/arm-none-eabi
    mv thumb/{nofp,v6-m} .
    rm -rf thumb/*
    mv nofp v6-m thumb
    cd -
rm -rf share/{doc,info,man,gdb,gcc-arm-none-eabi}
rm -rf bin/{arm-none-eabi-gdb,arm-none-eabi-lto-dump,arm-none-eabi-gfortran}

echo OK

