# TeRa command-line tool GUI

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

### 1. Install dependencies

#### Build tools

OSX

    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev

Windows

    * [Visual Studio Express 2013 for Windows Desktop] (https://www.microsoft.com/en-us/download/details.aspx?id=44914)

Detailed instructions for building dependencies are given as referense.

In Ubuntu and OSX it is assumed that user s is used for building and downloaded and unpacked to ~/Downloads and builds are done under ~/cmake_builds. In Windows the respective directories are c:\Downloads c:\cmake_builds. Change the directory names as necessary.

In Unutu and OSX building tools are available straight from terminal. In Windows use command prompt that opens from "VS2013 x86 Native Tools Command Prompt" from "Start"-menu -> "Visual Studio 2013" -> "Visual Studio Tools".

#### Install Qt5

https://www.qt.io/download-open-source/ (for Windows VS 2013 version is needed)

#### Install cmake

In Ubuntu

     sudo apt-get install cmake

OSX. Building from source https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

    ./bootstrap
    make
    sudo make install

For Windows install the latest .msi from https://cmake.org/download/

#### Install zlib

Download http://zlib.net/zlib-1.2.8.tar.gz

Ubuntu & OSX

    mkdir -p /home/s/cmake_builds/zlib & cd /home/s/cmake_builds/zlib
    cmake -G "Unix Makefiles" /home/s/Downloads/zlib-1.2.8
    cmake --build .
    sudo make install

In Windows

    cd c:\cmake_builds\zlib
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\zlib_bin -G "Visual Studio 12 2013" C:\Downloads\zlib-1.2.8
    "C:\Program Files\CMake\bin\cmake" --build .
    msbuild INSTALL.vcxproj

#### Install libzip

https://nih.at/libzip/libzip-1.1.3.tar.gz

Ubuntu & OSX

    mkdir -p /home/s/cmake_builds/libzip & cd /home/s/cmake_builds/libzip
    cmake -G "Unix Makefiles" /home/s/Downloads/libzip-1.1.3
    cmake --build .
    sudo make install

In Windows

    cd c:\cmake_builds\libzip
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\libzip_bin -G "Visual Studio 12 2013" C:\Downloads\libzip-1.1.3 -DCMAKE_PREFIX_PATH="C:\cmake_builds\zlib_bin"
    "C:\Program Files\CMake\bin\cmake" --build .
    msbuild INSTALL.vcxproj

#### Install openssl
https://www.openssl.org/source/openssl-1.1.0b.tar.gz See https://wiki.openssl.org/index.php/Compilation_and_Installation for details.

Short version for Ubuntu & OSX.

    ./config --openssldir=/usr/local/ssl
    make
    make test
    sudo make install

In Windows extract to C:\cmake_builds\openssl-1.1.0b. And install http://www.activestate.com/ActivePerl first. On casual windows command prompt dmake has to be installed for perl

    ppm install dmake

Then it is possible to build OpenSSL itself in Visual Studio command line (if 64-bit perl is used then first command may be 'set "PATH=%PATH%;C:\Perl64\bin"' instead)
    
    set "PATH=%PATH%;C:\Perl\bin"
    cd C:\cmake_builds\openssl-1.1.0b
    perl Configure VC-WIN32 no-asm --prefix=C:\cmake_builds\openssl-1.1.0b-bin --openssldir=C:\cmake_builds\openssl-1.1.0b-openssl
    nmake
    nmake install

#### Install boost

In Ubuntu

    sudo apt-get install libboost-all-dev

In OSX. Building from source http://www.boost.org/users/download/

    sudo apt-get install libdrm-dev
    ./bootstrap
    ./b2

In Windows. Building from source http://www.boost.org/users/download/. Extract source to C:\cmake_builds\boost_1_62_0 (version number may vary)

    cd C:\cmake_builds\boost_1_62_0
    bootstrap.bat
    b2

### 2. Configure

In Unbuntu & OSX

    mkdir -p /home/s/cmake_builds/tera & cd /home/s/cmake_builds/tera
    export CMAKE_PREFIX_PATH=/home/s/Qt/5.7/gcc_64/lib/cmake/
    cmake -G "Unix Makefiles" /home/s/git/TeRa

In Windows

    cd C:\cmake_builds\tera
    set "TERA_LIBZIP_DIR=C:\cmake_builds\libzip_bin"
    set "TERA_OPENSSL_DIR=C:\cmake_builds\openssl-1.1.0b-bin"
    set "CMAKE_PREFIX_PATH=C:\Qt5.7.0VS\5.7\msvc2013\lib\cmake;C:\cmake_builds\boost_1_62_0;C:\cmake_builds\zlib_bin;%TERA_LIBZIP_DIR%;%TERA_OPENSSL_DIR%"
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio 12 2013" C:\Downloads\git\TeRa

### 3. Build

In Ubuntu & OSX

    cmake --build .

In Windows

    "C:\Program Files\CMake\bin\cmake" --build . --config Release

### 4. Execute

In Ubuntu & OSX

    ./TeRaPOC

In Windows

    Release\TeRaPOC

    cd Release
    set QT_QPA_PLATFORM_PLUGIN_PATH=.
    TeRa.exe

PS The following dll's need to be copied to Release directory
Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, Qt5Widgets.dll, d3dcompiler_47.dll, libEGL.dll, libGLESv2.dll, opengl32sw.dll (C:\Qt5.7.0VS\5.7\msvc2013\bin),
qminimal.dll, qoffscreen.dll, qwindows.dll (C:\Qt5.7.0VS\5.7\msvc2013\plugins\platforms),
msvcp120.dll, msvcp120_clr0400.dll, msvcp120d.dll, msvcr120.dll, msvcr120_clr0400.dll, msvcr120d.dll (C:\Windows\SysWOW64),
libcrypto-1_1.dll, libssl-1_1.dll, libzip.dll, libzlib.dll, zip.dll and zlibd.dll
