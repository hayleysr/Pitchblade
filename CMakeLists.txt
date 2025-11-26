# Specify minimum CMake version supported. Project tested on 4.1.1
cmake_minimum_required(VERSION 3.22)

# Project name
project(Pitchblade)

set(CMAKE_CXX_STANDARD 23)

# Enable caching to make repeat builds run faster. Remove for complete rebuild.
# set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/cpm_cache")
# set(ENV{CPM_SOURCE_CACHE} "${CMAKE_BINARY_DIR}/cpm_cache")

# Store Juce lib files in /libs
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
file(MAKE_DIRECTORY ${LIB_DIR}) 

# Gently guide GTest and GMock to its own headers
set(gtest_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/gtest-src) 
# set(gmock_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/gtest-src)
# set(gtest_include_dirs "${gtest_SOURCE_DIR}/googletest/include")

# Try to find Juce locally
find_package(JUCE 8.0.9 QUIET)

if(NOT JUCE_FOUND)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpm.cmake)

    # Connect Juce
    CPMAddPackage(
        NAME JUCE 
        GITHUB_REPOSITORY juce-framework/JUCE 
        GIT_TAG 8.0.9
        VERSION v8.0.9
        SOURCE_DIR ${LIB_DIR}/juce
    )

    # Add Google Test
    CPMAddPackage(
        NAME gtest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.14.0  # Or any recent tag
        VERSION 1.14.0
        OPTIONS 
            "INSTALL_GTEST OFF"
            "gtest_disable_pthreads ON"
            "gtest_force_shared_crt ON" 
    )

    # If google test is being unreasonable
    target_include_directories(gtest PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googlemock/include>
    )
    target_include_directories(gmock PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googlemock/include>
    )
    target_include_directories(gtest PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest>
    )
    target_include_directories(gmock PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest>
    )
    target_include_directories(gtest_main PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googlemock/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest>
    )
    target_include_directories(gmock_main PUBLIC 
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googlemock/include>
        $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/googletest>
    )

endif()

# Report all warnings as errors
if(MSVC)
    #add_compile_options(/Wall /WX)
    #add_compile_options(/wd4514)
    add_compile_options(/W0)    # i don't want to see them.
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Link /plugin and run its CMake file
add_subdirectory(plugin)

# Enable testing for the whole project
enable_testing()

# Add the new tests directory
add_subdirectory(tests)