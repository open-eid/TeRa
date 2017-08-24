#!/bin/bash
# Requires xcode and cmake, also OpenSSL has to be installed via homebrew.
# Python must be present in path - it is available in macOS

######### Versions of libraries/frameworks to be compiled
ZLIB_VER="1.2.11"
LIBZIP_VER="1.1.3"
QT_VER="5.8.0"
CMAKE_OSX_DEPLOYMENT_TARGET="10.10.5"
#########


set -e

REBUILD=false
NO_QT=false
TERA_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH=~/cmake_builds
PROG=$0
TARGETS=""

while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
    -n|--no-qt)
        NO_QT=true
        ;;
    -o|--openssl)
        OPENSSL_PATH="$2"
        shift
        ;;
    -p|--path)
        BUILD_PATH="$2"
        shift
        ;;
    -q|--qt)
        QT_VER="$2"
        shift
        ;;
    -r|--rebuild)
        REBUILD=true
        ;;
    libzip|zlib|qt)
        TARGETS="$TARGETS,$key"
        ;;
    -h|--help)
        echo "Build dependencies (zlib, libzip and optionally Qt) of TeRa project"
        echo ""
        echo "Usage: $PROG [-r|--rebuild] [-p build-path] [-h|--help] [targets]"
        echo ""
        echo "Options:"
        echo "  -n or --no-qt:"
        echo "     Do not build Qt locally, use homebrew version"
        echo "  -o or --openssl openssl-path:"
        echo "     OpenSSL path; default found from homebrew"
        echo "  -p or --path build-path"
        echo "     folder where the dependencies should be built; default ~/cmake_builds"
        echo "  -q or --qt qt-version:"
        echo "     Specific version of Qt to build; default 5.8.0 "
        echo "  -r or --rebuild:"
        echo "     Rebuild even if dependency is already built"
        echo "  targets:"
        echo "     zlib libzip qt; default zlib libzip; "
        echo "     If qt is not defined, the script shall use the Qt installed by homebrew"
        echo " "
        echo "  -h or --help:"
        echo "     Print usage of the script "
        exit 0
        ;;
    esac
    shift # past argument or value
done

if [ -z "$TARGETS" ] ; then
    TARGETS="zlib,libzip,qt"
fi
if [ "$NO_QT" = true ] ; then
    TARGETS="${TARGETS//,qt}"
fi

if [ -z "$OPENSSL_PATH" ] ; then
    OPENSSL_PATH=`ls -d /usr/local/Cellar/openssl/1.0.* | tail -n 1`
fi
qt_ver_parts=( ${QT_VER//./ } )
QT_MINOR="${qt_ver_parts[0]}.${qt_ver_parts[1]}"

ZLIB_PATH=${BUILD_PATH}/zlib_bin
LIBZIP_PATH=${BUILD_PATH}/libzip_bin
QT_PATH=${BUILD_PATH}/Qt-${QT_VER}-OpenSSL

GREY='\033[0;37m'
ORANGE='\033[0;33m'
RED='\033[0;31m'
RESET='\033[0m'


if [[ $TARGETS == *"zlib"* ]] && [[ "$REBUILD" = true || ! -d ${ZLIB_PATH} ]] ; then
    echo -e "\n${ORANGE}##### Building zlib #####${RESET}\n"
    mkdir -p ${BUILD_PATH} && cd ${BUILD_PATH}
    curl -O -L http://zlib.net/zlib-${ZLIB_VER}.tar.gz
    tar xf zlib-${ZLIB_VER}.tar.gz
    mkdir -p ${BUILD_PATH}/zlib && cd ${BUILD_PATH}/zlib
    cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${ZLIB_PATH} -G "Unix Makefiles" ../zlib-${ZLIB_VER}
    cmake --build .
    make install
    rm -rf ${BUILD_PATH}/zlib
    rm -rf ${BUILD_PATH}/zlib-${ZLIB_VER}
    rm ${BUILD_PATH}/zlib-${ZLIB_VER}.tar.gz
else
    echo -e "\n${GREY}  zlib not built${RESET}"
fi

if [[ $TARGETS == *"libzip"* ]] && [[ "$REBUILD" = true || ! -d ${LIBZIP_PATH} ]] ; then
    echo -e "\n${ORANGE}##### Building libzip #####${RESET}\n"
    mkdir -p ${BUILD_PATH} && cd ${BUILD_PATH}
    curl -O -L https://nih.at/libzip/libzip-${LIBZIP_VER}.tar.gz
    tar xf libzip-${LIBZIP_VER}.tar.gz
    python $TERA_PATH/mac/libzip_static.py libzip-${LIBZIP_VER}/lib/CMakeLists.txt
    mkdir -p ${BUILD_PATH}/libzip && cd ${BUILD_PATH}/libzip
    export CMAKE_PREFIX_PATH=${ZLIB_PATH}
    cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${LIBZIP_PATH} -G "Unix Makefiles" ../libzip-${LIBZIP_VER}
    cmake --build .
    make install
    rm -rf ${BUILD_PATH}/libzip
    rm -rf ${BUILD_PATH}/libzip-${LIBZIP_VER}
    rm ${BUILD_PATH}/libzip-${LIBZIP_VER}.tar.gz
else
    echo -e "\n${GREY}  libzip not built${RESET}"
fi

if [[ $TARGETS == *"qt"* ]] && [[ "$REBUILD" = true || ! -d ${QT_PATH} ]] ; then
    echo -e "\n${ORANGE}##### Building Qt #####${RESET}\n"
    mkdir -p ${BUILD_PATH} && cd ${BUILD_PATH}
    curl -O -L http://download.qt.io/official_releases/qt/${QT_MINOR}/${QT_VER}/submodules/qtbase-opensource-src-${QT_VER}.tar.gz
    tar xf qtbase-opensource-src-${QT_VER}.tar.gz
    cd qtbase-opensource-src-${QT_VER}
    ./configure -prefix ${QT_PATH} -opensource -nomake tests -nomake examples -no-securetransport -openssl-linked -confirm-license -I /usr/local/opt/openssl/include -L /usr/local/opt/openssl/lib
    make
    make install
    rm -rf ${BUILD_PATH}/qtbase-opensource-src-${QT_VER}
    rm ${BUILD_PATH}/qtbase-opensource-src-${QT_VER}.tar.gz

    cd ${BUILD_PATH}
    curl -O -L http://download.qt.io/official_releases/qt/${QT_MINOR}/${QT_VER}/submodules/qttools-opensource-src-${QT_VER}.tar.gz
    tar xf qttools-opensource-src-${QT_VER}.tar.gz
    cd qttools-opensource-src-${QT_VER}
    "${QT_PATH}"/bin/qmake
    make
    make install
elif [ "$NO_QT" = true ] ; then
    QT_PATH=`ls -d /usr/local/Cellar/qt/5.* | tail -n 1`
    if [ -z "$QT_PATH" ] ; then
        echo -e "\n${RED}  Homebrew Qt not found${RESET}"
        exit 1
    fi
    echo -e "\n${GREY}  Using homebrew Qt ${QT_PATH}${RESET}"
else
    echo -e "\n${GREY}  Qt not built${RESET}"
fi

echo "# Load TERA build variables:
# . ./env.sh or source ./env.sh
export TERA_QT_DIR=${QT_PATH}
export TERA_LIBZIP_DIR=${LIBZIP_PATH}
export TERA_ZLIB_DIR=${ZLIB_PATH}
export TERA_OPENSSL_DIR=${OPENSSL_PATH}
export CMAKE_PREFIX_PATH=$QT_PATH/lib/cmake:$TERA_ZLIB_DIR:$TERA_LIBZIP_DIR:$TERA_OPENSSL_DIR
" > $TERA_PATH/env.sh
chmod u+x $TERA_PATH/env.sh