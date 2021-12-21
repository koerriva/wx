add_executable(DummyGame DummyGame.cpp)
include_directories(${PROJECT_SOURCE_DIR}/engine/include)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)
target_link_libraries(DummyGame PRIVATE spdlog::spdlog engine)