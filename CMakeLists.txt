# Project
    cmake_minimum_required(VERSION 3.25)
    project(VideoDecoder)

# Settings
    set(CMAKE_CXX_STANDARD 23)                  
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

# Resources Files
    file(COPY ${CMAKE_SOURCE_DIR}/placeholders DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/_resources DESTINATION ${CMAKE_BINARY_DIR})

# Install CPM
    set(CPM_USE_LOCAL_PACKAGES ON)

    set(CPM_DOWNLOAD_VERSION 0.42.0)
    set(CPM_HASH_SUM "2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a")

    if(CPM_SOURCE_CACHE)
        set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    elseif(DEFINED ENV{CPM_SOURCE_CACHE})
        set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    else()
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    endif()

    # Expand relative path. This is important if the provided path contains a tilde (~)
        get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )

    include(${CPM_DOWNLOAD_LOCATION})

# Boost-asio repository
    CPMAddPackage(
        NAME asiocmake
        GIT_REPOSITORY "https://github.com/OlivierLDff/asio.cmake"
        GIT_TAG "main"
    )

# Packages
    find_package(FFMPEG REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

# QT Setup
    qt_standard_project_setup()

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

# Sources
    file(GLOB_RECURSE SRCS 
        "src/*.cpp" 
        "src/**/*.cpp"
        "include/*.hpp"
        "include/**/*.hpp"
    )

# Executable
    qt_add_executable(${PROJECT_NAME} src/main.cpp ${CMAKE_SOURCE_DIR}/_resources/res.qrc)

# Configuration into the last version of the C++
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Target the file sources
    target_sources(${PROJECT_NAME} PRIVATE ${SRCS} _resources/icon.rc)

# Target the directories
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/include 
        ${CMAKE_SOURCE_DIR}/resources
        ${FFMPEG_INCLUDE_DIRS}
    )

# Include the FFMPEG lib's directories
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})

# Include the FFMPEG, Winsock, Asio libs
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network)
    target_link_libraries(${PROJECT_NAME} PUBLIC asio::asio)

# Deploy Qt for Windows
    if(WIN32)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND Qt6::windeployqt
            ARGS
                --no-translations
                --no-system-d3d-compiler
                $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Deploying Qt plugins (platforms, styles, imageformats...)"
        )
    endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
endif()