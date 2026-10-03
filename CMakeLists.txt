cmake_minimum_required(VERSION 3.25)
project(DigForever LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)           # standard C++17, no compiler-specific extensions

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---------------------------------------------------------------------------
# raylib: downloaded and compiled automatically (no Git or install needed)
# ---------------------------------------------------------------------------
include(FetchContent)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   # lets raylib's bundled libraries build on CMake 4
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)   # hides raylib's old-CMake warning
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM                              # warnings from raylib's headers are not shown
)
FetchContent_MakeAvailable(raylib)

# ---------------------------------------------------------------------------
# The game: every .cpp in DigForever/src is compiled automatically
# ---------------------------------------------------------------------------
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/DigForever/src/*.cpp")
if (NOT GAME_SOURCES)
    message(FATAL_ERROR "No .cpp files found in ${CMAKE_SOURCE_DIR}/DigForever/src. "
                        "Put this CMakeLists.txt in the repo root, next to README.md.")
endif()

add_executable(main ${GAME_SOURCES})
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/DigForever/src")
target_link_libraries(main PRIVATE raylib)

# ---------------------------------------------------------------------------
# Warnings (your code only)
# ---------------------------------------------------------------------------
if (MSVC)
    target_compile_options(main PRIVATE
        /W4             # all reasonable warnings
        /permissive-    # strict standard conformance
        /w14242         # conversion, possible loss of data
        /w14254         # larger bit field assigned to smaller one
        /w14263         # member function hides base virtual function
        /w14265         # class has virtual functions but non-virtual destructor
        /w14287         # unsigned/negative constant mismatch
        /we4289         # loop variable used outside the loop (error)
        /w14296         # expression is always true/false
        /w14311         # pointer truncation
        /w14545 /w14546 /w14547 /w14549   # suspicious comma expressions
        /w14555         # expression has no effect
        /w14619         # unknown warning number in #pragma warning
        /w14640         # thread-unsafe static local initialization
        /w14826         # sign-extending conversion
        /w14905 /w14906 # wrong string literal cast
        /w14928         # illegal copy-initialization
    )
    if (WARNINGS_AS_ERRORS)
        target_compile_options(main PRIVATE /WX)
    endif()
else()
    target_compile_options(main PRIVATE
        -Wall -Wextra -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wcast-align
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wunused
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(main PRIVATE
            -Wuseless-cast -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wmisleading-indentation)
    endif()
    if (WARNINGS_AS_ERRORS)
        target_compile_options(main PRIVATE -Werror)
    endif()
endif()

# Put main.exe in the project root
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>")