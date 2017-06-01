# TeRa command-line tool GUI

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

* [Ubuntu](#ubuntu)
* [OS X](#osx)
* [Windows](#windows)



### Ubuntu

#### 1. Install dependencies

    sudo apt-get update
    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev git cmake zlib1g zlib1g-dev libpcsclite-dev libssl-dev qtbase5-dev qttools5-dev qttools5-dev-tools libzip-dev pcscd

#### 2. Fetch the source

    mkdir -p ~/cmake_builds/github && cd ~/cmake_builds/github
    git clone https://github.com/open-eid/TeRa.git

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

#### 1. Install dependencies

###### a) Build tools

    install xcode
    Qt5 https://www.qt.io/download-open-source/ TODO see back
    TODO git

###### b) CMake

Building from source https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

    ./bootstrap
    make
    sudo make install
    
###### c) zlib

Download and install the latest zlib from http://zlib.net/

    mkdir -p ~/cmake_builds/ && cd ~/cmake_builds/
    curl -O -L http://zlib.net/zlib-1.2.11.tar.gz
    tar xf zlib-1.2.11.tar.gz
    mkdir -p ~/cmake_builds/zlib && cd ~/cmake_builds/zlib
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/zlib_bin -G "Unix Makefiles" ../zlib-1.2.11
    cmake --build .
    make install

###### d) libzip

Download and install the latest version from https://nih.at/libzip/
(NB! Do make static version of the libraries as well uncomment lines at the end of the file libzip-1.1.3/lib/CMakeList.txt before running cmake (starting from "#ADD_LIBRARY(zipstatic STATIC ...").)

    mkdir -p ~/cmake_builds/ && cd ~/cmake_builds/
    curl -O -L https://nih.at/libzip/libzip-1.1.3.tar.gz
    tar xf libzip-1.1.3.tar.gz
    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    export CMAKE_PREFIX_PATH=$HOME/cmake_builds/zlib_bin
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ../libzip-1.1.3
    cmake --build .
    make install

###### e) OpenSSL

Install OpenSSL from https://www.openssl.org/source/openssl-1.0.2h.tar.gz (for details, see https://wiki.openssl.org/index.php/Compilation_and_Installation#OS_X)
See http://stackoverflow.com/questions/41865537/how-does-apples-codesign-utility-decide-which-sha-algorithms-to-sign-a-shared and https://wiki.openssl.org/index.php/Compilation_and_Installation (look for "-mios-version-min=")

    mkdir -p ~/cmake_builds/ && cd ~/cmake_builds/
    curl -O -L https://www.openssl.org/source/openssl-1.0.2h.tar.gz
    tar xf openssl-1.0.2h.tar.gz
    cd ~/cmake_builds/openssl-1.0.2h
    ./Configure darwin64-x86_64-cc shared --openssldir=$HOME/cmake_builds/openssl-1.0.2h.bin -mmacosx-version-min=10.10
    make depend
    make install
    make test


###### f) Qt with OpenSSL support


    export C_INCLUDE_PATH=$HOME/cmake_builds/openssl-1.0.2h.bin/include/
    export CPLUS_INCLUDE_PATH=$C_INCLUDE_PATH
    export OPENSSL_PATH="$HOME/cmake_builds/openssl-1.0.2h.bin/"
    export OPENSSL_LIBS="-L$HOME/cmake_builds/openssl-1.0.2h.bin/lib/ -lssl -lcrypto"
    curl -O -L http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.tar.gz
    tar xf qtbase-opensource-src-5.8.0.tar.gz
    cd qtbase-opensource-src-5.8.0
    ./configure -prefix ~/Qt5.8.0-OpenSSL -openssl-linked -opensource -nomake tests -nomake examples -no-securetransport -confirm-license
    make
    make install
    cd ..
    rm -rf qtbase-opensource-src-5.8.0

    curl -O -L http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qttools-opensource-src-5.8.0.tar.gz
    tar xf qttools-opensource-src-5.8.0.tar.gz
    cd qttools-opensource-src-5.8.0
    ~/Qt5.8.0-OpenSSL/bin/qmake
    make
    make install
    cd ..
    rm -rf qttools-opensource-src-5.8.0

#### 2. Fetch the source

    mkdir -p ~/cmake_builds/github && cd ~/cmake_builds/github
    git clone https://github.com/open-eid/TeRa.git

#### 3. Configure

    mkdir -p ~/cmake_builds/tera && cd ~/cmake_builds/tera
    export TERA_QT_DIR=$HOME/Qt5.8.0-OpenSSL
    export TERA_LIBZIP_DIR=$HOME/cmake_builds/libzip_bin
    export TERA_ZLIB_DIR=$HOME/cmake_builds/zlib_bin
    export TERA_OPENSSL_DIR=$HOME/cmake_builds/openssl-1.0.2h.bin
    export CMAKE_PREFIX_PATH=$TERA_QT_DIR/lib/cmake:$TERA_ZLIB_DIR:$TERA_LIBZIP_DIR:$TERA_OPENSSL_DIR
    cmake -G "Unix Makefiles" ~/cmake_builds/github/TeRa

#### 4. Build

    cmake --build .

#### 5. TODO Install

To include Qt libraries to TeRa.app

    ~/Qt5.8.0-OpenSSL/bin/macdeployqt TeRa.app/

#### 6. TODO Execute

    TODO ./TeRaPOC




### Windows

#### 1. Install dependencies

###### a) Build tools

    * [Visual Studio Express 2013 for Windows Desktop] (https://www.microsoft.com/en-us/download/details.aspx?id=44914)

Detailed instructions for building dependencies are given as referense.

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

    Release\TeRa.exe        // for GUI
    Release\TeRaTool.exe    // for command line

PS The following dll's need to be copied to Release directory
Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, Qt5Widgets.dll, d3dcompiler_47.dll, libEGL.dll, libGLESv2.dll, opengl32sw.dll (C:\Qt5.7.0VS\5.7\msvc2013\bin),
qwindows.dll to Release\platforms (C:\Qt5.7.0VS\5.7\msvc2013\plugins\platforms),
msvcp120.dll, msvcr120.dll (C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\),

### 5. Creating installer

Commands to build .msi

    "C:\Program Files\CMake\bin\cmake" --build . --target run_wix_candle
    "C:\Program Files\CMake\bin\cmake" --build . --target run_wix_light
