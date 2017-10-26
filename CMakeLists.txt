#
#  K-PEG - Simple Basline JPEG Encoder & Decoder
# ===============================================
#
# CMake build script

cmake_minimum_required(VERSION 3.0)
project(kpeg)

# Add directory containing FindSFML.cmake to module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

# Add source
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Specify include directory
include_directories("${PROJECT_SOURCE_DIR}/include/")

# Find SFML
if (SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
    find_package(SFML 2 COMPONENTS main audio graphics window system REQUIRED)
else()
    find_package(SFML 2 COMPONENTS audio graphics window system REQUIRED)
endif()

# Found SFML
if(SFML_FOUND)
        include_directories(${SFML_INCLUDE_DIR})
else()
        set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
        message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
        message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

# Compile and generate the executable
add_executable(kpeg main.cpp src/KPEG.cpp src/Encoder.cpp src/Decoder.cpp src/ImageViewer.cpp src/Image.cpp src/Logger.cpp src/HuffmanTree.cpp src/MCU.cpp) #${SOURCES})

target_link_libraries(kpeg ${SFML_LIBRARIES})

set_property(TARGET kpeg PROPERTY CXX_STANDARD 14)
set_property(TARGET kpeg PROPERTY CXX_STANDARD_REQUIRED ON)

#install(TARGETS kpeg RUNTIME DESTINATION bin)
