cmake_minimum_required(VERSION 3.17)
project(TicTacToe C)

set(CMAKE_C_STANDARD 11)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(server server.c)
add_executable(server_states server_states.c)
target_link_libraries(server_states dcfsm)