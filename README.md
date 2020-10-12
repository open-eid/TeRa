# DEPRECATED TeRa command-line tool GUI

![European Regional Development Fund](/images/reg_logo.png "European Regional Development Fund")

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

[![Build Status](https://travis-ci.com/open-eid/TeRa.svg?branch=master)](https://travis-ci.com/open-eid/TeRa)

* [Ubuntu](#ubuntu)
* [OS X](#osx)
* [Windows](#windows)



### Ubuntu

#### 1. Install dependencies

    sudo apt-get update
    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev git cmake zlib1g zlib1g-dev libpcsclite-dev libssl-dev qtbase5-dev qttools5-dev qttools5-dev-tools libzip-dev pcscd

#### 2. Fetch the source

    mkdir -p ~/cmake_builds/github && cd ~/cmake_builds/github
    git clone --recursive https://github.com/open-eid/TeRa.git

#### 3. Configure

    mkdir -p ~/cmake_builds/tera_build && cd ~/cmake_builds/tera_build
    cmake -G "Unix Makefiles" ~/cmake_builds/github/TeRa

#### 4. Configure

    cmake --build .

#### 5. Install

    sudo make install

#### 6. Execute

Command line tool:

    /usr/local/bin/qdigidoc-tera

GUI:

    /usr/local/bin/qdigidoc-tera-gui



### OSX

#### 1. Install build tools

Install [XCode](https://itunes.apple.com/ee/app/xcode/id497799835?mt=12)
Download CMake binary package from https://cmake.org or install via homebrew.

#### 2. Install project dependencies

    brew install openssl qt
    ./prepare_osx_build_environment.sh

NB! The preparation script downloads and compiles Qt locally. In order to use homebrew version of Qt run the preparation script with parameters: `./prepare_osx_build_environment.sh --no-qt`

Alternatively you can install all dependencies manually, see [instructions for manual build](macos-manual.md).

#### 3. Fetch the source

    git clone --recursive https://github.com/open-eid/TeRa.git

#### 3. Configure

    cd TeRa
    . ./env.sh # Created by preparation script prepare_osx_build_environment.sh
    mkdir build && cd build 
    cmake ..

#### 4. Build

    make

#### 5. TODO Install

To include Qt libraries to TeRa.app

    ~/Qt5.8.0-OpenSSL/bin/macdeployqt TeRa.app/

#### 6. Execute

    open qdigidoc-tera.app




### Windows

#### 1. Install dependencies

###### a) Build tools

    * [Visual Studio Express 2013 for Windows Desktop] (https://www.microsoft.com/en-us/download/details.aspx?id=44914)

Detailed instructions for building dependencies are given as reference.

In Windows the respective directories are c:\Downloads c:\cmake_builds. Change the directory names as necessary.

Use command prompt that opens from "VS2013 x86 Native Tools Command Prompt" from "Start"-menu -> "Visual Studio 2013" -> "Visual Studio Tools".

    set "TERA_ARCH_C=12 2013"
    set "TERA_ARCH=_32"
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.8.0\5.8\msvc2013"

PS For x64 builds "VS2013 x64 Cross..." and 'cmake -G "Visual Studio 12 2013 Win64" ...' need to be used instead

    set "TERA_ARCH_C=12 2013 Win64"
    set "TERA_ARCH=_64"
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.8.0_64\5.8\msvc2013_64"

###### b) CMake

Install the latest .msi from https://cmake.org/download/

###### c) zlib

Download latest zlib from http://zlib.net/ and untar it to C:\Downloads\ (ex. http://zlib.net/zlib-1.2.11.tar.gz)

    mkdir c:\cmake_builds\zlib%TERA_ARCH%
    cd c:\cmake_builds\zlib%TERA_ARCH%
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\zlib_bin%TERA_ARCH% -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\zlib-1.2.8
    "C:\Program Files\CMake\bin\cmake" --build . --config Release
    msbuild INSTALL.vcxproj /property:Configuration=Release

###### d) libzip

Download from https://nih.at/libzip/libzip-1.1.3.tar.gz and untar to C:\Downloads\.

Enable static build of libzip - open C:\Downloads\libzip-1.1.3\lib\CMakeList.txt and uncomment lines at the end of the file (starting from "#ADD_LIBRARY(zipstatic STATIC ...").

    mkdir c:\cmake_builds\libzip%TERA_ARCH%
    cd c:\cmake_builds\libzip%TERA_ARCH%
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\libzip_bin%TERA_ARCH% -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\libzip-1.1.3 -DCMAKE_PREFIX_PATH="C:\cmake_builds\zlib_bin%TERA_ARCH%"
    "C:\Program Files\CMake\bin\cmake" --build . --config Release
    msbuild INSTALL.vcxproj /property:Configuration=Release

###### e) OpenSSL

Download https://www.openssl.org/source/openssl-1.0.2h.tar.gz (See https://wiki.openssl.org/index.php/Compilation_and_Installation for details)

In Windows extract to C:\cmake_builds\openssl-1.1.0b(_32/_64). And install http://www.activestate.com/ActivePerl first. On casual windows command prompt dmake has to be installed for perl

    ppm install dmake

Then it is possible to build OpenSSL itself in Visual Studio command line (if 64-bit perl is used then first command may be 'set "PATH=%PATH%;C:\Perl64\bin"' instead)
    
    set "PATH=%PATH%;C:\Perl\bin"

Win 32 version:

    cd C:\cmake_builds\openssl-1.0.2h_32
    perl Configure VC-WIN32 no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll_32
    ms\do_ms
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install

Win 64 version:

    cd C:\cmake_builds\openssl-1.0.2h_64
    perl Configure VC-WIN64A no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll_64
    ms\do_win64a
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install

###### f) Qt

https://www.qt.io/download-open-source/ (5.8.0 Windows VS 2013 (x86/x64) version is needed)

###### g) Wix - for creating installer only

Download fromhttp://wixtoolset.org/releases/ and extracted so that C:\Downloads\wix310-binaries\bin\candle.exe points to valid executable.



### 2. Configure

Open Visual Stuido 2013 command line and set up parameters as described in "Windows" -> "1. Install dependencies" -> "a) Build tools"

    mkdir C:\cmake_builds\tera%TERA_ARCH%
    cd C:\cmake_builds\tera%TERA_ARCH%
    set "WIX_PATH=C:\Downloads\wix310-binaries\"
    set "TERA_LIBZIP_DIR=C:\cmake_builds\libzip_bin%TERA_ARCH%"
    set "TERA_ZLIB_DIR=C:\cmake_builds\zlib_bin%TERA_ARCH%"
    set "TERA_OPENSSL_DIR=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%"
    set "CMAKE_PREFIX_PATH=%TERA_QT_CMAKE_DIR%\lib\cmake;%TERA_ZLIB_DIR%;%TERA_LIBZIP_DIR%;%TERA_OPENSSL_DIR%"
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\git\TeRa

### 3. Build

    "C:\Program Files\CMake\bin\cmake" --build . --config Release

### 4. Execute

In Windows

    Release\qdigidoc_tera_gui.exe   // for GUI
    Release\qdigidoc_tera.exe       // for command line

PS The following dll's need to be copied to Release directory
Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, Qt5Widgets.dll, d3dcompiler_47.dll, libEGL.dll, libGLESv2.dll, opengl32sw.dll (C:\Qt5.7.0VS\5.7\msvc2013\bin),
qwindows.dll to Release\platforms (C:\Qt5.7.0VS\5.7\msvc2013\plugins\platforms),
msvcp120.dll, msvcr120.dll (C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\),

### 5. Creating installer

Commands to build .msi

    "C:\Program Files\CMake\bin\cmake" --build . --target run_wix_candle
    "C:\Program Files\CMake\bin\cmake" --build . --target run_wix_light
