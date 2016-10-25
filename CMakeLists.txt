cmake_minimum_required (VERSION 2.6)

project (TeRa)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find Qt libraries
find_package(Qt5Core)
include_directories(${Qt5Core_INCLUDE_DIRS})
find_package(Qt5Network)
include_directories(${Qt5Network_INCLUDE_DIRS})
find_package(Qt5Concurrent)
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

#set(libzip_DIR 'C:\\Program Files\\libzip')
#find_package(libzip PATHS )

# C++11
set(CMAKE_CXX_STANDARD 14)

# TeRaPOC
add_executable(TeRaPOC poc/terapoc.cpp poc/terapoc.h)
# dll'i jaoks
target_link_libraries(TeRaPOC zipdll)
qt5_use_modules(TeRaPOC Core Concurrent Network)

