# Project
    cmake_minimum_required(VERSION 3.25)
    project(VideoDecoder)

# Settings
    set(CMAKE_CXX_STANDARD 23)                  
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

# Resources Files
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# FFMPEG package
    find_package(FFMPEG REQUIRED)

# Sources
    file(GLOB_RECURSE SRCS 
        "src/*.cpp" 
        "src/**/*.cpp"
        "include/*.hpp"
        "include/**/*.hpp"
    )

# Executable
    add_executable(${PROJECT_NAME} src/main.cpp)

# Configuration into the last version of the C++
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Target the file sources
    target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

# Target the directories
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/include 
        ${CMAKE_SOURCE_DIR}/resources
        ${FFMPEG_INCLUDE_DIRS}
    )

# Include the FFMPEG lib's directories
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})

# Include the FFMPEG & Winsock libs
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32)
