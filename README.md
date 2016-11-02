## Building (Ubuntu & OSX)

### 1. Install dependencies

#### Install cmake

In Ubuntu

   sudo apt-get install cmake

Building from source https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

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
see https://wiki.openssl.org/index.php/Compilation_and_Installation

    ./config --openssldir=/usr/local/ssl
    make
    make test
    sudo make install

#### Install boost

In Ubuntu

    sudo apt-get install libboost-all-dev

Building from source http://www.boost.org/users/download/

    ./bootstrap
    ./b2

### 2. Configure

mkdir -p /home/s/cmake_builds/tera & cd /home/s/cmake_builds/tera
export CMAKE_PREFIX_PATH=/home/s/Qt/5.7/gcc_64/lib/cmake/
cmake -G "Unix Makefiles" /home/s/git/TeRa

### 3. Build

    cmake --build .

### 4. Execute

    ./TeRaPOC