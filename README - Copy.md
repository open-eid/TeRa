## Building (Ubuntu & OSX)

### 1. Install dependencies

For Windows Microsoft Visual Studio Express 2013 for Windows Desktop is neede

#### Install Qt5

TODO

#### Install cmake

In Ubuntu

   sudo apt-get install cmake

Building from source (OSX) https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

  ./bootstrap
  make
  sudo make install

#### Install zlib

http://zlib.net/zlib-1.2.8.tar.gz

    mkdir -p /home/s/cmake_builds/zlib & cd /home/s/cmake_builds/zlib
    cmake -G "Unix Makefiles" /home/s/Downloads/zlib-1.2.8
    cmake --build .
    sudo make install

#### Install libzip

https://nih.at/libzip/libzip-1.1.3.tar.gz

    mkdir -p /home/s/cmake_builds/libzip & cd /home/s/cmake_builds/libzip
    cmake -G "Unix Makefiles" /home/s/Downloads/libzip-1.1.3
    cmake --build .
    sudo make install

#### Install openssl
https://www.openssl.org/source/openssl-1.1.0b.tar.gz
see https://wiki.openssl.org/index.php/Compilation_and_Installation for details. Short version

    ./config --openssldir=/usr/local/ssl
    make
    make test
    sudo make install

#### Install boost

In Ubuntu

    sudo apt-get install libboost-all-dev

Building from source (OSX) http://www.boost.org/users/download/

    ./bootstrap
    ./b2

    
    
    
    >>>>>>>> sudo apt-get install libdrm-dev
    sudo apt-get install build-essential libgl1-mesa-dev
    
    
    
### 2. Configure

mkdir -p /home/s/cmake_builds/tera & cd /home/s/cmake_builds/tera
export CMAKE_PREFIX_PATH=/home/s/Qt/5.7/gcc_64/lib/cmake/
cmake -G "Unix Makefiles" /home/s/git/TeRa

### 3. Build

    cmake --build .

### 4. Execute

    ./TeRaPOC


======================================================================================================================================================
https://github.com/open-eid/qdigidoc



## Building (Windows)


[NEW] https://www.microsoft.com/en-us/download/details.aspx?id=44914
[NEW] http://download.qt.io/official_releases/qt/5.7/5.7.0/qt-opensource-windows-x86-msvc2013-5.7.0.exe


[NEW] "VS2013 x86 Native Tools Command Prompt"
      %comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"" x86
      "C:\Program Files\CMake\bin\cmake"


Install MSYS2

Install development tools & Qt5 (https://wiki.qt.io/MSYS2, packman commands may take quite a long time ~1h to execute)
   Initial Setup of MSYS2
   Prepare MSYS2 for Qt related build development environment
   Obtain Pre-Built Qt & QtCreator binary files and Use instantly without Building/Compiling


### install cmake
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-i686-cmake

### zlib & libzip easy


[NEW]
    mkdir -p /home/s/cmake_builds/zlib & cd /home/s/cmake_builds/zlib
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio 12 2013" C:\work\TeRa\lugemist\zlib-1.2.8
    "C:\Program Files\CMake\bin\cmake" --build .

    msbuild INSTALL.vcxproj
                //// /P:Configuration=Release 

    http://stackoverflow.com/questions/10507893/libzip-with-visual-studio-2010
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio 12 2013" C:\work\TeRa\lugemist\libzip-1.1.3 -DCMAKE_PREFIX_PATH="C:\Program Files (x86)\zlib"

    zconf.h line 454 ("#    include <unistd.h>")
    #ifdef _WIN32
    #include <io.h>
    #else
    #include <unistd.h>
    #endif

[NEW]
Boost
 unzip & bootstrap.bat & .\b2

   
            The Boost C++ Libraries were successfully built!

            The following directory should be added to compiler include paths:

                C:\cmake_builds\boost_1_62_0

            The following directory should be added to linker library paths:

                C:\cmake_builds\boost_1_62_0\stage\lib

[NEW]
    #### Install openssl
    http://www.askyb.com/windows/compiling-and-installing-openssl-for-32-bit-windows/
    https://wiki.openssl.org/index.php/Compilation_and_Installation#W32_.2F_Windows_NT_-_Windows_9x
        http://www.activestate.com/ActivePerl
    
    
    set "PATH=%PATH%;C:\Perl\bin"
    ppm install dmake
    perl Configure VC-WIN32 no-asm --prefix=C:\cmake_builds\openssl-1.1.0b-bin
    nmake
    nmake install
    
    
[NEW] Tera
  TODO C:\cmake_builds\libzip\zipconf.h to C:\Program Files (x86)\libzip\include
    cd C:\cmake_builds\tera
    set "TERA_LIBZIP_DIR=C:\Program Files (x86)\libzip"
    set "TERA_OPENSSL_DIR=C:\cmake_builds\openssl-1.1.0b-bin"
    set "CMAKE_PREFIX_PATH=C:\Qt5.7.0VS\5.7\msvc2013\lib\cmake;C:\cmake_builds\boost_1_62_0;C:\Program Files (x86)\zlib;%TERA_LIBZIP_DIR%;%TERA_OPENSSL_DIR%"
    "C:\Program Files\CMake\bin\cmake" -G "Visual Studio 12 2013" C:\work\TeRa\git\TeRa
    "C:\Program Files\CMake\bin\cmake" --build .
    "C:\Program Files\CMake\bin\cmake" --build . --config Release






https://www.visualstudio.com/vs/visual-studio-express/

C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin\SetEnv.cmd


https://github.com/open-eid/libdigidocpp/blob/master/README.md


cd C:\work\TeRa\cmake_build\tera


https://www.microsoft.com/en-us/download/details.aspx?id=4422

C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin\SetEnv.cmd
cmake -G "Visual Studio 14 2015 Win64" C:\work\TeRa\git\TeRa


cmake -G "MSYS Makefiles" /home/tammeojak/git/tera
and no "sudo" in front of "make install"
installs into Program Files

### openssl
https://wiki.qt.io/Compiling_OpenSSL_with_MinGW

./Configure --openssldir=/usr/local/ssl no-idea no-mdc2 no-rc5 shared mingw
make depend && make && make install




pacman -S mingw-w64-x86_64-boost mingw-w64-i686-boost   ;;;;

/bin/bash bootstrap.sh mingw
./b2
## bootstrap mingw
## b2 toolset=gcc 




