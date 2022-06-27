include_directories(${PROJECT_SOURCE_DIR}/engine/include)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)
include_directories(${PROJECT_SOURCE_DIR}/spdlog-1.x/include)

add_executable(DummyGame DummyGame.cpp)
target_link_libraries(DummyGame PRIVATE spdlog::spdlog engine)

add_executable(GLTFViewer GLTFViewer.cpp)
target_link_libraries(GLTFViewer engine)

add_executable(PlanetTest PlanetTest.cpp)
target_link_libraries(PlanetTest PRIVATE spdlog::spdlog engine)
