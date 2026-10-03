cmake_minimum_required(VERSION 3.20)
project(DigForever LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# raylib: downloaded and compiled automatically
include(FetchContent)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)

# The game: every .cpp in DigForever/src is compiled automatically
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/DigForever/src/*.cpp")

add_executable(main ${GAME_SOURCES})
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/DigForever/src")
target_link_libraries(main PRIVATE raylib)

if (MSVC)
    target_compile_options(main PRIVATE /W4)
else()
    target_compile_options(main PRIVATE -Wall -Wextra)
endif()

# Put main.exe in the project root
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>")
