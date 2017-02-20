# TeRa command-line tool GUI

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

* [Ubuntu](#ubuntu)
* [OS X](#osx)
* [Windows](#windows)

### Ubuntu

#### 1. Install dependencies

1. Build tools

    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev git-all cmake zlib1g zlib1g-dev libpcsclite-dev libssl-dev

2. Qt 5.6+. Download and install Qt from https://www.qt.io/download-open-source/

    chmod +x qt-opensource-linux-x64-5.8.0.run
    ./qt-opensource-linux-x64-5.8.0.run


3. libzip 1.1.x+. Download and install from https://nih.at/libzip/

Download:

    mkdir -p ~/cmake_builds && cd ~/cmake_builds
    wget -O https://nih.at/libzip/libzip-1.1.3.tar.gz
    tar xzvf libzip-1.1.3.tar.gz

Build:

    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ./libzip-1.1.3
    cmake --build .
    make install

#### 2. Fetch the source

    cd ~/cmake_builds
    git clone https://github.com/open-eid/TeRa.git

#### 3. Configure

    export CMAKE_PREFIX_PATH=$HOME/Qt5.8.0/5.8/gcc_64/lib/cmake:$HOME/cmake_builds/libzip_bin
    cmake -G "Eclipse CDT4 - Unix Makefiles" ~/git/TeRa
    cmake -G "Unix Makefiles" ~/git/TeRa

#### 4. Configure

    cmake --build .

#### 5. TODO Install

#### 6. TODO Execute

    TODO ./TeRaPOC




TODO For OSX and Windows enable static build of libzip open C:\Downloads\libzip-1.1.3\lib\CMakeList.txt and uncommend lines at the end of the file (starting from "#ADD_LIBRARY(zipstatic STATIC ...").

Ubuntu & OSX

    mkdir -p ~/cmake_builds/libzip && cd ~/cmake_builds/libzip
    wget -O https://nih.at/libzip/libzip-1.1.3.tar.gz
    export CMAKE_PREFIX_PATH=$HOME/cmake_builds/zlib_bin
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/libzip_bin -G "Unix Makefiles" ~/Downloads/libzip-1.1.3
    cmake --build .
    make install


    git clone https://github.com/open-eid/TeRa.git

    export CMAKE_PREFIX_PATH=$HOME/Qt5.8.0/5.8/gcc_64/lib/cmake:$HOME/cmake_builds/libzip_bin
    cmake -G "Eclipse CDT4 - Unix Makefiles" ~/git/TeRa
    cmake -G "Unix Makefiles" ~/git/TeRa
    cmake --build .



    sudo apt-get install eclipse eclipse-cdt


    
    
    

http://doc.qt.io/qt-5/osx-building.html
export OPENSSL_PATH="$HOME/cmake_builds/openssl-1.0.2h.bin/"
export OPENSSL_LIBS="-L$HOME/cmake_builds/openssl-1.0.2h.bin/lib/ -lssl -lcrypto"
./configure -openssl-linked -opensource -nomake examples -nomake tests -prefix ~/Qt5.8.0-OpenSSL
make
make -j1 install


>>>>>>>>>>>>>>>>>>>>>>>>
    
    
    mkdir -p ~/cmake_builds/tera_src && cd ~/cmake_builds/tera_src
    git clone https://github.com/open-eid/TeRa.git

    mkdir -p ~/cmake_builds/tera && cd ~/cmake_builds/tera
    
#  TERA_UNIX_CUSTOM_LIBS
#    export TERA_LIBZIP_DIR=$HOME/cmake_builds/libzip_bin
#    export TERA_ZLIB_DIR=$HOME/cmake_builds/zlib_bin
#    export TERA_OPENSSL_DIR=$HOME/cmake_builds/openssl-1.0.2h.bin
#    export CMAKE_PREFIX_PATH=$HOME/Qt5.8.0/5.8/gcc_64/lib/cmake:$TERA_ZLIB_DIR:$TERA_LIBZIP_DIR:$TERA_OPENSSL_DIR
#    cmake -G "Unix Makefiles" ~/cmake_builds/tera_src/TeRa -DOPENSSL_ROOT_DIR=$TERA_OPENSSL_DIR









    zlib libzip

??? In Ubuntu
???
???     sudo apt-get install cmake



>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#### Build tools

OSX

    install xcode
    TODO git

Windows

    * [Visual Studio Express 2013 for Windows Desktop] (https://www.microsoft.com/en-us/download/details.aspx?id=44914)

Detailed instructions for building dependencies are given as referense.

In Ubuntu and OSX it is assumed that user s is used for building and downloaded and unpacked to ~/Downloads and builds are done under ~/cmake_builds. In Windows the respective directories are c:\Downloads c:\cmake_builds. Change the directory names as necessary.

In Unutu and OSX building tools are available straight from terminal. In Windows use command prompt that opens from "VS2013 x86 Native Tools Command Prompt" from "Start"-menu -> "Visual Studio 2013" -> "Visual Studio Tools".

    set "TERA_ARCH_C=12 2013"
    set "TERA_ARCH=_32"
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.8.0\5.8\msvc2013"
    

PS For x64 builds "VS2013 x64 Cross..." and 'cmake -G "Visual Studio 12 2013 Win64" ...' need to be used instead

    set "TERA_ARCH_C=12 2013 Win64"
    set "TERA_ARCH=_64"
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.8.0_64\5.8\msvc2013_64"

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

Short version for Ubuntu & OSX. Untar tar.gz to ~/cmake_builds

    # cd ~/cmake_builds
    # tar xzvf ~/Downloads/openssl-1.1.0b.tar.gz
    # cd openssl-1.1.0b 
    # ./config --prefix=$HOME/cmake_builds/openssl-1.1.0b-bin --openssldir=$HOME/cmake_builds/openssl-1.1.0b-openssl
    # make
    # make test
    # make install


MacOS (for details, see https://wiki.openssl.org/index.php/Compilation_and_Installation#OS_X)

    tar xxx
    ./Configure darwin64-x86_64-cc shared --openssldir=$HOME/cmake_builds/openssl-1.0.2h.bin
    make depend
    make install
    make test


In Windows extract to C:\cmake_builds\openssl-1.1.0b(_32/_64). And install http://www.activestate.com/ActivePerl first. On casual windows command prompt dmake has to be installed for perl

    ppm install dmake

Then it is possible to build OpenSSL itself in Visual Studio command line (if 64-bit perl is used then first command may be 'set "PATH=%PATH%;C:\Perl64\bin"' instead)
    
    set "PATH=%PATH%;C:\Perl\bin"

    cd C:\cmake_builds\openssl-1.0.2h%TERA_ARCH%
    perl Configure VC-WIN32 no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
    ms\do_ms
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install
    

    cd C:\cmake_builds\openssl-1.0.2h%TERA_ARCH%
    perl Configure VC-WIN64A no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
    ms\do_win64a
    nmake -f ms\ntdll.mak
    nmake -f ms\ntdll.mak test
    nmake -f ms\ntdll.mak install

### 2. Configure

In OSX

    mkdir -p ~/cmake_builds/tera_src && cd ~/cmake_builds/tera_src
    git clone https://github.com/open-eid/TeRa.git

    mkdir -p ~/cmake_builds/tera && cd ~/cmake_builds/tera
    export TERA_LIBZIP_DIR=$HOME/cmake_builds/libzip_bin
    export TERA_ZLIB_DIR=$HOME/cmake_builds/zlib_bin
    export TERA_OPENSSL_DIR=$HOME/cmake_builds/openssl-1.0.2h.bin
    export CMAKE_PREFIX_PATH=$HOME/Qt5.7.0/5.7/clang_64/lib/cmake:$TERA_ZLIB_DIR:$TERA_LIBZIP_DIR:$TERA_OPENSSL_DIR
    cmake -G "Unix Makefiles" ~/cmake_builds/tera_src/TeRa -DOPENSSL_ROOT_DIR=$TERA_OPENSSL_DIR

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
