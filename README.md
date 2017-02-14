# TeRa command-line tool GUI

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building

### 1. Install dependencies

#### Build tools

Ubuntu

    sudo apt-get install libdrm-dev build-essential libgl1-mesa-dev git-all

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
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.5.1\5.5\msvc2013"
    

PS For x64 builds "VS2013 x64 Cross..." and 'cmake -G "Visual Studio 12 2013 Win64" ...' need to be used instead

    set "TERA_ARCH_C=12 2013 Win64"
    set "TERA_ARCH=_64"
    set "TERA_QT_CMAKE_DIR=C:\Qt\Qt5.8.0_64\5.8\msvc2013_64"

#### Install Qt5

https://www.qt.io/download-open-source/ (Windows VS 2013 (x86) version is needed; Digidoc uses currently 5.5.1, so this version is preferred for compatibility)

#### Install cmake

In Ubuntu

     sudo apt-get install cmake

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
https://www.openssl.org/source/openssl-1.1.0b.tar.gz See https://wiki.openssl.org/index.php/Compilation_and_Installation for details.

Short version for Ubuntu & OSX. Untar tar.gz to ~/cmake_builds

    cd ~/cmake_builds
    tar xzvf ~/Downloads/openssl-1.1.0b.tar.gz
    cd openssl-1.1.0b 
    ./config --prefix=$HOME/cmake_builds/openssl-1.1.0b-bin --openssldir=$HOME/cmake_builds/openssl-1.1.0b-openssl
    make
    make test
    make install

In Windows extract to C:\cmake_builds\openssl-1.1.0b(_32/_64). And install http://www.activestate.com/ActivePerl first. On casual windows command prompt dmake has to be installed for perl

    ppm install dmake

Then it is possible to build OpenSSL itself in Visual Studio command line (if 64-bit perl is used then first command may be 'set "PATH=%PATH%;C:\Perl64\bin"' instead)
    
    set "PATH=%PATH%;C:\Perl\bin"
    cd C:\cmake_builds\openssl-1.1.0b%TERA_ARCH%
  cd C:\cmake_builds\openssl-1.0.2h%TERA_ARCH%
    //perl Configure VC-WIN32 no-asm no-shared --prefix=C:\cmake_builds\openssl-1.1.0b-bin%TERA_ARCH% --openssldir=C:\cmake_builds\openssl-1.1.0b-openssl%TERA_ARCH%
      or for 64 bit...
       perl Configure VC-WIN64A no-asm no-shared --prefix=C:\cmake_builds\openssl-1.1.0b-bin%TERA_ARCH% --openssldir=C:\cmake_builds\openssl-1.1.0b-openssl%TERA_ARCH%
  perl Configure VC-WIN64A no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
     // --openssldir=C:\cmake_builds\openssl-1.0.2h-openssl%TERA_ARCH%
    nmake
    nmake install


http://www.microsoft.com/msdownload/platformsdk/sdkupdate/

perl Configure VC-WIN64A no-asm --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
perl Configure VC-WIN64A --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
ms\do_ms
nmake -f ms\ntdll.mak

   
   
        link /nologo /subsystem:console /opt:ref /debug /dll /out:out32dll\libeay32.dll /def:ms/LIBEAY32.def @C:\Users\TAMMEO~1\AppData\Local\Temp\nm261F.tmp
LINK : fatal error LNK1181: cannot open input file 'unicows.lib'
NMAKE : fatal error U1077: '"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\BIN\x86_amd64\link.EXE"' : return code '0x49d'
   C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib

SET "LIB=%LIB%;C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib"


http://www.colinkraft.com/mame/mame.php


nmake -f ms\ntdll.mak test
nmake -f ms\ntdll.mak install






perl Configure VC-WIN64A --prefix=C:\cmake_builds\openssl-1.0.2h-bin-dll%TERA_ARCH%
ms\do_win64a
nmake -f ms\ntdll.mak
cd out32dll
..\ms\test
cd ..
nmake -f ms\ntdll.mak install






### 2. Configure

In Unbuntu & OSX

    mkdir -p ~/cmake_builds/tera_src && cd ~/cmake_builds/tera_src
    git clone https://github.com/open-eid/TeRa.git

    mkdir -p ~/cmake_builds/tera && cd ~/cmake_builds/tera
    export CMAKE_PREFIX_PATH=$HOME/Qt5.7.1/5.7/gcc_64/lib/cmake:$HOME/cmake_builds/zlib_bin:$HOME/cmake_builds/libzip_bin
    cmake -G "Unix Makefiles" ~/cmake_builds/tera_src/TeRa

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
    "%WIX%\candle.exe" tera.wxs -dMSI_VERSION=0.7.0 -dqt_path=%TERA_QT_CMAKE_DIR% -dclient_path=%TERA_BUILD_DIR%\Release -dPlatform=%TERA_WIX_PLATFORM%
    "%WIX%\light.exe" -out TeRa%TERA_ARCH%.msi tera.wixobj -v -ext WixUIExtension
