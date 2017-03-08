# TeRa command-line tool GUI

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

* [Ubuntu](#ubuntu)
* [OS X](#osx)
* [Windows](#windows)



### Ubuntu

#### 1. Install dependencies

###### a) Build tools

    sudo apt-get update
    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev git-all cmake zlib1g zlib1g-dev libpcsclite-dev libssl-dev

###### b) Qt 5.6+.

Download and install Qt from https://www.qt.io/download-open-source/

    wget http://download.qt.io/official_releases/qt/5.8/5.8.0/qt-opensource-linux-x64-5.8.0.run
    chmod +x qt-opensource-linux-x64-5.8.0.run
    ./qt-opensource-linux-x64-5.8.0.run

###### c) libzip 1.1.x+.

Download and install from https://nih.at/libzip/ (only latest wersion is kept on the page, if the link does not exist find newer link to download "Distfiles: ..., libzip-X.X.X.tar.gz")

    mkdir -p ~/cmake_builds && cd ~/cmake_builds
    wget https://nih.at/libzip/libzip-1.1.3.tar.gz
    tar xzvf libzip-1.1.3.tar.gz

Build:

    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ../libzip-1.1.3
    cmake --build .
    make install

###### d) ID-card software for drivers

https://installer.id.ee/?lang=est

#### 2. Fetch the source

    mkdir -p ~/cmake_builds/github && cd ~/cmake_builds/github
    git clone https://github.com/open-eid/TeRa.git

#### 3. Configure

    mkdir -p ~/cmake_builds/tera_build && cd ~/cmake_builds/tera_build
    export CMAKE_PREFIX_PATH=$HOME/Qt5.8.0/5.8/gcc_64/lib/cmake:$HOME/cmake_builds/libzip_bin
    cmake -G "Unix Makefiles" ~/cmake_builds/github/TeRa

#### 4. Configure

    cmake --build .

#### 5. TODO Install

#### 6. TODO Execute

    TODO ./TeRaPOC



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
(Do make static version of the libraries as well uncommend lines at the end of the file libzip-1.1.3/lib/CMakeList.txt before running cmake (starting from "#ADD_LIBRARY(zipstatic STATIC ...").)

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

    mkdir -p ~/cmake_builds/ && cd ~/cmake_builds/
    curl -O -L https://www.openssl.org/source/openssl-1.0.2h.tar.gz
    tar xf openssl-1.0.2h.tar.gz
    cd ~/cmake_builds/openssl-1.0.2h
    ./Configure darwin64-x86_64-cc shared --openssldir=$HOME/cmake_builds/openssl-1.0.2h.bin
    make depend
    make install
    make test


###### f) Qt with OpenSSL support


    export C_INCLUDE_PATH=$HOME/cmake_builds/openssl-1.0.2h.bin/include/
    export OPENSSL_PATH="$HOME/cmake_builds/openssl-1.0.2h.bin/"
    export OPENSSL_LIBS="-L$HOME/cmake_builds/openssl-1.0.2h.bin/lib/ -lssl -lcrypto"
    curl -O -L http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.tar.gz
    tar xf qtbase-opensource-src-5.8.0.tar.gz
    cd qtbase-opensource-src-5.8.0
./configure -prefix ~/Qt5.8.0-OpenSSL-dyn -openssl -opensource -nomake tests -nomake examples -no-securetransport -confirm-license
    ./configure -prefix ~/Qt5.8.0-OpenSSL -openssl-linked -opensource -nomake tests -nomake examples -no-securetransport -confirm-license
    make
    sudo make install
    cd ..
    rm -rf qtbase-opensource-src-5.8.0

    curl -O -L http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qttools-opensource-src-5.8.0.tar.gz
    tar xf qttools-opensource-src-5.8.0.tar.gz
    cd qttools-opensource-src-5.8.0
    ~/Qt5.8.0-OpenSSL/bin/qmake
    make
    sudo make install
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



/////////////////////////////////////////////////////////////////////


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






For OSX and Windows enable static build of libzip open C:\Downloads\libzip-1.1.3\lib\CMakeList.txt and uncommend lines at the end of the file (starting from "#ADD_LIBRARY(zipstatic STATIC ...").

Ubuntu & OSX

    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    wget -O https://nih.at/libzip/libzip-1.1.3.tar.gz
    export CMAKE_PREFIX_PATH=$HOME/cmake_builds/zlib_bin
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ~/Downloads/libzip-1.1.3
    cmake --build .
    make install


    git clone https://github.com/open-eid/TeRa.git

    export CMAKE_PREFIX_PATH=$HOME/Qt5.8.0/5.8/gcc_64/lib/cmake:$HOME/cmake_builds/libzip_bin
    cmake -G "Unix Makefiles" ~/git/TeRa
    cmake --build .







>>>>>>>>>>>>>>>>>>>>>>>>
    
    
    mkdir -p ~/cmake_builds/tera_src && cd ~/cmake_builds/tera_src
    git clone https://github.com/open-eid/TeRa.git

    mkdir -p ~/cmake_builds/tera && cd ~/cmake_builds/tera
    
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#### Build tools


#### Install Qt5

https://www.qt.io/download-open-source/ (Windows VS 2013 (x86) version is needed; Digidoc uses currently 5.5.1, so this version is preferred for compatibility)

#### Install cmake

OSX. Building from source https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

    ./bootstrap
    make
    sudo make install

For Windows install the latest .msi from https://cmake.org/download/

#### Install zlib

Download latest zlib from http://zlib.net/ and untar it to ~/Downloads (ex. http://zlib.net/zlib-1.2.11.tar.gz)

Ubuntu & OSX

    mkdir -p ~/cmake_builds/zlib && cd ~/cmake_builds/zlib
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/zlib_bin -G "Unix Makefiles" ~/Downloads/zlib-1.2.11
    cmake --build .
    make install

In Windows

    mkdir c:\cmake_builds\zlib%TERA_ARCH%
    cd c:\cmake_builds\zlib%TERA_ARCH%
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\zlib_bin%TERA_ARCH% -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\zlib-1.2.8
    "C:\Program Files\CMake\bin\cmake" --build . --config Release
    msbuild INSTALL.vcxproj /property:Configuration=Release

#### Install libzip

https://nih.at/libzip/libzip-1.1.3.tar.gz

For OSX and Windows enable static build of libzip open C:\Downloads\libzip-1.1.3\lib\CMakeList.txt and uncommend lines at the end of the file (starting from "#ADD_LIBRARY(zipstatic STATIC ...").

Ubuntu & OSX

    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    export CMAKE_PREFIX_PATH=$HOME/cmake_builds/zlib_bin
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ~/Downloads/libzip-1.1.3
    cmake --build .
    make install

In Windows

    mkdir c:\cmake_builds\libzip%TERA_ARCH%
    cd c:\cmake_builds\libzip%TERA_ARCH%
    "C:\Program Files\CMake\bin\cmake" -DCMAKE_INSTALL_PREFIX=c:\cmake_builds\libzip_bin%TERA_ARCH% -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\libzip-1.1.3 -DCMAKE_PREFIX_PATH="C:\cmake_builds\zlib_bin%TERA_ARCH%"
    "C:\Program Files\CMake\bin\cmake" --build . --config Release
    msbuild INSTALL.vcxproj /property:Configuration=Release

#### Install openssl
https://www.openssl.org/source/openssl-1.0.2h.tar.gz See https://wiki.openssl.org/index.php/Compilation_and_Installation for details.

In Windows extract to C:\cmake_builds\openssl-1.1.0b(_32/_64). And install http://www.activestate.com/ActivePerl first. On casual windows command prompt dmake has to be installed for perl

    ppm install dmake

Then it is possible to build OpenSSL itself in Visual Studio command line (if 64-bit perl is used then first command may be 'set "PATH=%PATH%;C:\Perl64\bin"' instead)
    
    set "PATH=%PATH%;C:\Perl\bin"

    cd C:\cmake_builds\openssl-1.0.2h_32
    perl Configure VC-WIN32 no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll_32
    ms\do_ms
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install
    

    cd C:\cmake_builds\openssl-1.0.2h_64
    perl Configure VC-WIN64A no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll_64
    ms\do_win64a
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install

### 2. Configure

In Windows (put in your Qt path in "set "CMAKE_PREFIX_PATH=...")

    mkdir C:\cmake_builds\tera%TERA_ARCH%
    cd C:\cmake_builds\tera%TERA_ARCH%
    set "TERA_LIBZIP_DIR=C:\cmake_builds\libzip_bin%TERA_ARCH%"
    set "TERA_ZLIB_DIR=C:\cmake_builds\zlib_bin%TERA_ARCH%"
    set "TERA_OPENSSL_DIR=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%"
    set "CMAKE_PREFIX_PATH=%TERA_QT_CMAKE_DIR%\lib\cmake;%TERA_ZLIB_DIR%;%TERA_LIBZIP_DIR%;%TERA_OPENSSL_DIR%"
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio %TERA_ARCH_C%" C:\Downloads\git\TeRa

### 3. Build

In Ubuntu & OSX

    cmake --build .

For OSX to include Qt libraries to TeRa.app

    ~/Qt5.7.0/5.7/clang_64/bin/macdeployqt TeRa.app/

In Windows

    "C:\Program Files\CMake\bin\cmake" --build . --config Release

### 4. Execute

In Ubuntu & OSX

    ./TeRaPOC

In Windows

    Release\TeRaPOC

    cd Release
    TeRa.exe        // for GUI
    TeRaTool.exe    // for command line

PS The following dll's need to be copied to Release directory
Qt5Core.dll, Qt5Gui.dll, Qt5Network.dll, Qt5Widgets.dll, d3dcompiler_47.dll, libEGL.dll, libGLESv2.dll, opengl32sw.dll (C:\Qt5.7.0VS\5.7\msvc2013\bin),
qwindows.dll to Release\platforms (C:\Qt5.7.0VS\5.7\msvc2013\plugins\platforms),
msvcp120.dll, msvcr120.dll (C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\),

## Installer

Windows

    * Wix is needed to create .msi (http://wixtoolset.org/releases/ - following commands assume binaries are downloaded and extracted so that C:\Downloads\wix310-binaries\bin\candle.exe points to valid executable)

Commands to build .msi (check QT path)

    Same set of "set"-commands as given under "Build tools", plus...
        set TERA_WIX_PLATFORM=x86
    or
        set TERA_WIX_PLATFORM=x64

    cd C:\Downloads\git\TeRa
    set "WIX=C:\Downloads\wix310-binaries"
    set "TERA_BUILD_DIR=C:\cmake_builds\tera%TERA_ARCH%"
    "%WIX%\candle.exe" tera.wxs -dMSI_VERSION=0.7.0 -dqt_path=%TERA_QT_CMAKE_DIR% -dopenssl_path=%TERA_OPENSSL_DIR% -dclient_path=%TERA_BUILD_DIR%\Release -dPlatform=%TERA_WIX_PLATFORM%
    "%WIX%\light.exe" -out TeRa%TERA_ARCH%.msi tera.wixobj -v -ext WixUIExtension
