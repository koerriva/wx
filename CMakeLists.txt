cmake_minimum_required(VERSION 3.21)
project(wx)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(glad)
add_subdirectory(noise)

file(COPY data DESTINATION ${PROJECT_BINARY_DIR}/game)