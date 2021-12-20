cmake_minimum_required(VERSION 3.21)
project(wx)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(glad)

file(COPY data DESTINATION .)