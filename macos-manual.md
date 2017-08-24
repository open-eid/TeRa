### OSX

#### 1. Install dependencies

##### a) Build tools

Install [XCode](https://itunes.apple.com/ee/app/xcode/id497799835?mt=12)

##### b) CMake

Download CMake binary package from https://cmake.org or install via homebrew.

##### c) zlib

Download and install the latest zlib from http://zlib.net/

    mkdir -p ~/cmake_builds/ && cd ~/cmake_builds/
    curl -O -L http://zlib.net/zlib-1.2.11.tar.gz
    tar xf zlib-1.2.11.tar.gz
    mkdir -p ~/cmake_builds/zlib && cd ~/cmake_builds/zlib
    cmake -DCMAKE_INSTALL_PREFIX=~/cmake_builds/zlib_bin -G "Unix Makefiles" ../zlib-1.2.11
    cmake --build .
    make install

##### d) libzip

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

##### e) OpenSSL

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


##### f) Qt with OpenSSL support


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

#### 6. Execute

    open qdigidoc-tera.app