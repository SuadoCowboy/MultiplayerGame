cmake_minimum_required(VERSION 3.10.0)
project(MultiplayerGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Client
add_executable(MultiplayerGame src/Client.cpp src/Network.cpp src/TimeSystem.cpp src/Math.cpp)

target_include_directories(MultiplayerGame PRIVATE include src)
target_link_libraries(MultiplayerGame PRIVATE -L${PROJECT_SOURCE_DIR}/libs -lenet-d -lws2_32 -lwinmm -lraylib)

# Server
add_executable(MultiplayerGameServer src/Server.cpp src/Network.cpp src/ClientsHandler.cpp src/TimeSystem.cpp)

target_include_directories(MultiplayerGameServer PRIVATE include src)
target_link_libraries(MultiplayerGameServer PRIVATE -L${PROJECT_SOURCE_DIR}/libs -lenet-d -lws2_32 -lwinmm -lraylib)