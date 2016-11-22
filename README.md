## Building (Ubuntu & OSX)

### 1. Install dependencies

#### Install Qt5

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




## Building (Windows)

Install MSYS2

Install development tools & Qt5 (https://wiki.qt.io/MSYS2, packman commands may take quite a long time ~1h to execute)
   Initial Setup of MSYS2
   Prepare MSYS2 for Qt related build development environment
   Obtain Pre-Built Qt & QtCreator binary files and Use instantly without Building/Compiling


### install cmake
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-i686-cmake

### zlib & libzip easy


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




