# === To update translation files ===
# (for further reading see https://itk.org/Wiki/CMake:How_To_Build_Qt4_Software)
# * Uncomment "SET (UPDATE_TRANSLATIONS 1)"
# * Build configuration with cmake
# * Build with cmake
# * NB! Comment "SET (UPDATE_TRANSLATIONS 1)" out again when not needed anymore and build new configuration with cmake
# (* Open&translate translations/*.ts with "Qt Linquist")
# (* at the end "File -> Release All" to generate binary versions that are compiled into executables)
# (* Build with cmake)
#SET (UPDATE_TRANSLATIONS 1)

cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0020 NEW)
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
endif()

project (TeRa)
IF (UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "compile flags" FORCE)
ENDIF()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON) 
SET(CMAKE_AUTORCC ON)

# Find Qt libraries
find_package(Qt5Core)
include_directories(${Qt5Core_INCLUDE_DIRS})
find_package(Qt5Network)
include_directories(${Qt5Network_INCLUDE_DIRS})
find_package(Qt5Concurrent)
include_directories(${Qt5Concurrent_INCLUDE_DIRS})
find_package(Qt5Gui)
find_package(Qt5Widgets) 
include_directories(${Qt5Widgets_INCLUDE_DIRS})

FIND_PACKAGE(Qt5LinguistTools)

#find_package(qdigidoccommon)
#SET (QTCOMMON_INCLUDE_DIRS "C:/Downloads/git/qt-common")
#SET (QTCOMMON_LIBRARIES "C:/cmake_builds/qt-common_64/Release/qdigidoccommon.lib")
#include_directories(${QTCOMMON_INCLUDE_DIRS})

IF (WIN32)
    #find_package(ZLib)
    SET (ZLIB_INCLUDE_DIRS "$ENV{TERA_ZLIB_DIR}/include")
    SET (ZLIB_LIBRARIES "$ENV{TERA_ZLIB_DIR}/lib/zlibstatic.lib")
    MESSAGE(${ZLIB_INCLUDE_DIRS})
    MESSAGE(${ZLIB_LIBRARIES})
    # libzip
    ADD_DEFINITIONS(-DZIP_STATIC)
    include_directories("$ENV{TERA_LIBZIP_DIR}/include" "$ENV{TERA_LIBZIP_DIR}/lib/libzip/include")
    SET (ZIP_LIBRARIES "$ENV{TERA_LIBZIP_DIR}/lib/zipstatic.lib")
    # for static openssl
    SET (FOR_OPENSSL_STATIC crypt32.lib ws2_32.lib) # GDI64.LIB ADVAPI64.LIB USER64.LIB)
ELSE()
    include_directories(/usr/local/lib/libzip/include/)
    IF (APPLE)
        SET (ZLIB_LIBRARIES $ENV{HOME}/cmake_builds/zlib/libz.a)
        SET (ZIP_LIBRARIES $ENV{HOME}/cmake_builds/libzip/lib/libzipstatic.a)
    ELSE()
        SET (ZLIB_LIBRARIES $ENV{HOME}/cmake_builds/zlib/libz.a)
        SET (ZIP_INCLUDE_DIR $ENV{HOME}/cmake_builds/libzip_bin/include $ENV{HOME}/cmake_builds/libzip_bin/lib/libzip/include/)
        include_directories( ${ZIP_INCLUDE_DIR} )
        SET (ZIP_LIBRARIES $ENV{HOME}/cmake_builds/libzip/lib/libzipstatic.a)
    ENDIF()
    SET (FOR_OPENSSL_STATIC)
ENDIF()

# include openssl
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_MSVC_STATIC_RT TRUE)
find_package(OpenSSL)
include_directories( ${OPENSSL_INCLUDE_DIR} )

message(STATUS "ZLIB_LIBRARIES:      " ${ZLIB_LIBRARIES})
message(STATUS "ZIP_LIBRARIES:       " ${ZIP_LIBRARIES})
message(STATUS "ZIP_INCLUDE_DIR:     " ${ZIP_INCLUDE_DIR})
message(STATUS "OPENSSL_INCLUDE_DIR: " ${OPENSSL_INCLUDE_DIR})
message(STATUS "OPENSSL_LIBRARIES:   " ${OPENSSL_LIBRARIES})

# C++11
set(CMAKE_CXX_STANDARD 14)

IF (WIN32)
ELSE()
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread" )
    IF (APPLE)
    ELSE()
        set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
    ENDIF()
ENDIF()

SET (TERA_COMMON_SRC
      src/version.h
     )

SET (TERA_LIBDIGIDOC_SRC
        src/libdigidoc/Configuration.h src/libdigidoc/Configuration.cpp
        src/libdigidoc/Settings.h)

SET (TERA_GUI_UI poc/MainWindow.ui poc/SettingsDialog.ui poc/FileListDialog.ui)
SET (TERA_GUI_SRC
        poc/xxx.cpp
        poc/main_window.h poc/main_window.cpp ComboBox.h ComboBox.cpp
        poc/gui_timestamper_processor.h poc/gui_timestamper_processor.cpp
        poc/settings_window.h poc/settings_window.cpp
        poc/files_window.h poc/files_window.cpp

        poc/utils.h poc/utils.cpp
        poc/openssl_utils.h poc/openssl_utils.cpp
        poc/disk_crawler.h poc/disk_crawler.cpp
        poc/logging.h poc/logging.cpp
        poc/timestamper.h poc/timestamper.cpp
        poc/config.h poc/config.cpp
     )

SET (TRANS translations/et.ts translations/en.ts translations/ru.ts)

if (UPDATE_TRANSLATIONS)
    qt5_create_translation( QM ${TERA_GUI_UI} ${TERA_GUI_SRC} ${TRANS})
ENDIF()
qt5_add_translation( QM ${TRANS} )
qt5_add_resources( CONFIG_SOURCES res/config.qrc )
qt5_add_resources( SOURCES images/images.qrc translations/tr.qrc )
qt5_wrap_ui( UI_HEADERS ${TERA_GUI_UI} )

# TeRaTool
add_executable(TeRaTool
        ${TERA_COMMON_SRC}
        poc/teragui.rc
        poc/terapoc.cpp
        poc/utils.h poc/utils.cpp
        poc/openssl_utils.h poc/openssl_utils.cpp
        poc/disk_crawler.h poc/disk_crawler.cpp
        poc/logging.h poc/logging.cpp
        poc/timestamper.h poc/timestamper.cpp
        poc/config.h poc/config.cpp
        ${CONFIG_SOURCES}
        )
target_link_libraries(TeRaTool ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${FOR_OPENSSL_STATIC} ${ZLIB_LIBRARIES} )
qt5_use_modules(TeRaTool Core Concurrent Network)

add_executable(TeRa WIN32 MACOSX_BUNDLE
        ${TERA_COMMON_SRC}
        poc/teragui.rc
        ${UI_HEADERS}
        ${TERA_GUI_UI}
        ${TERA_GUI_SRC}
        ${SOURCES}
        ${TERA_LIBDIGIDOC_SRC}
        ${CONFIG_SOURCES}
        ${QM}
        ${TRANS}
        )
target_link_libraries(TeRa ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${FOR_OPENSSL_STATIC} ${ZLIB_LIBRARIES})
qt5_use_modules(TeRa Core Concurrent Network Widgets Gui)
target_link_libraries(TeRa Qt5::Widgets)
