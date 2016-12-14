cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0020 NEW)
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
endif()

project (TeRa)
set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "compile flags" FORCE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON) 
SET(CMAKE_AUTORCC ON)

########### SET(UPDATE_TRANSLATIONS ON)

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

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS log_setup log thread date_time filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIR})
MESSAGE("Boost information")  
MESSAGE("Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")  
MESSAGE("Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")  
MESSAGE("Boost Libraries: ${Boost_LIBRARIES}") 

IF (WIN32)
    find_package(ZLib)
    include_directories("$ENV{TERA_LIBZIP_DIR}/include")
    find_library(libzip NAMES zip.lib PATHS "$ENV{TERA_LIBZIP_DIR}/lib")
    MESSAGE(${ZLIB_LIBRARIES})
    SET (ZIP_LIBRARIES "C:/Program Files (x86)/libzip/lib/zip.lib")
ELSE ()
    include_directories(/usr/local/lib/libzip/include/)
    find_library(libzip NAMES libzip.so PATHS /usr/local/lib/)
    SET (ZIP_LIBRARIES /home/s/Downloads/libzip-1.1.3/lib/.libs/libzip.a /home/s/cmake_builds/zlib/libz.a)
ENDIF()

# include openssl
find_package(OpenSSL)
include_directories( ${OPENSSL_INCLUDE_DIR} )
# TODO include_directories(/usr/local/include)
# TODO find_library(ssl NAMES libssl.so  PATHS /home/s/Downloads/openssl-1.1.0b)
# TODO find_library(crypto NAMES libcrypto.so PATHS /home/s/Downloads/openssl-1.1.0b)

#add_library(zip STATIC /home/s/Downloads/libzip-1.1.3/lib/libzip.la)

# C++11
set(CMAKE_CXX_STANDARD 14)

SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread" )

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

##### TODO To update translation files (see https://itk.org/Wiki/CMake:How_To_Build_Qt4_Software)
# * Uncomment qt5_create_translation
# * Build configuration with cmake
# * Build with cmake
# * NB! Comment qt5_create_translation out again when not needed anymore and build new configuration with cmake
#qt5_create_translation( QM ${TERA_GUI_UI} ${TERA_GUI_SRC} ${TRANS})

qt5_add_translation( QM ${TRANS} )
qt5_add_resources( SOURCES images/images.qrc translations/tr.qrc )
qt5_wrap_ui( UI_HEADERS ${TERA_GUI_UI} )

# TeRaPOC
add_executable(TeRaPOC
        poc/teragui.rc
        poc/terapoc.cpp
        poc/utils.h poc/utils.cpp
        poc/openssl_utils.h poc/openssl_utils.cpp
        poc/disk_crawler.h poc/disk_crawler.cpp
        poc/logging.h poc/logging.cpp
        poc/timestamper.h poc/timestamper.cpp
        poc/config.h poc/config.cpp
        )
# -lboost_filesystem -lboost_system
# ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY}
#target_link_libraries(TeRaPOC /home/s/Downloads/libzip-1.1.3/lib/.libs/libzip.a /home/s/cmake_builds/zlib/libz.a ssl crypto ${Boost_LIBRARIES} )
##### TODO zip
target_link_libraries(TeRaPOC ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} )
qt5_use_modules(TeRaPOC Core Concurrent Network)

add_executable(Tera #WIN32 MACOSX_BUNDLE
        poc/teragui.rc
        ${UI_HEADERS}
        ${TERA_GUI_UI}
        ${TERA_GUI_SRC}
        images/images.qrc translations/tr.qrc ${SOURCES}
        ${QM}
        ${TRANS}
        )
##### TODO zip
target_link_libraries(Tera ${ZIP_LIBRARIES} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} )
qt5_use_modules(Tera Core Concurrent Network Widgets Gui)
target_link_libraries(Tera Qt5::Widgets)