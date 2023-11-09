cmake_minimum_required(VERSION 3.27)

project (SoftwareRendering)

set(project_root ${CMAKE_SOURCE_DIR})

include_directories(
    ${project_root}/src/
    ${project_root}/vendor/STB
    ${project_root}/vendor/SDL/include
    ${project_root}/vendor/sse2neon/
    ${project_root}/vendor/thread-pool/include
    ${project_root}/vendor/ConcurrencyPrimitives/src
)

add_executable(${PROJECT_NAME} 
    ${project_root}/src/color.cpp
    ${project_root}/src/edge_function.cpp
    ${project_root}/src/line.cpp
    ${project_root}/src/mesh.cpp
    ${project_root}/src/point.cpp
    ${project_root}/src/vector.cpp
    ${project_root}/src/rect.cpp
    ${project_root}/src/texture.cpp
    ${project_root}/src/triangle.cpp
    ${project_root}/src/uv.cpp
    ${project_root}/src/camera.cpp
    ${project_root}/src/vertex.cpp
    ${project_root}/src/main.cpp
    # math
    ${project_root}/src/math/vec4f32.cpp
    ${project_root}/src/math/vec4i32.cpp
    ${project_root}/src/math/mat4f32.cpp
    # ConcurrencyPrimitives
    ${project_root}/vendor/ConcurrencyPrimitives/src/ThreadPool.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

set(SDL2_DIR ${project_root}/vendor/SDL/cmake)
find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2main)