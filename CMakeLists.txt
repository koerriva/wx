cmake_minimum_required(VERSION 3.6)
project(wx)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# 查找spdlog 【重要！！！，不查找将导致进入头文件模式，编译时间缓慢】
find_package(spdlog REQUIRED)

add_subdirectory(game)
add_subdirectory(engine)
add_subdirectory(glad)
add_subdirectory(noise)

file(COPY data DESTINATION ${PROJECT_BINARY_DIR}/game)