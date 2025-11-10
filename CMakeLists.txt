cmake_minimum_required(VERSION 3.20)
project(2048_game_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler options for better warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Source files - organized by module
set(SRC_FILES
        src/main.cpp
        src/Game/Game.cpp
        src/Game/Board.cpp
        src/Render/Renderer.cpp
        src/Input/InputHandler.cpp
        src/Game/Game.cpp
        src/Render/Renderer.cpp
        src/Input/InputHandler.cpp
)

# Include directories - so we can use #include "Game/Game.h" etc.
include_directories(
        src
        src/Game
        src/Render
        src/Input
        src/Utils
)

# Create the executable
add_executable(2048_game_cpp ${SRC_FILES})

# For macOS: prevent bundle creation for console app
if(APPLE)
    set_target_properties(2048_game_cpp PROPERTIES
            MACOSX_BUNDLE FALSE
    )
endif()