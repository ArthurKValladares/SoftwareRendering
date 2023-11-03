cmake_minimum_required(VERSION 3.27)

project (SoftwareRendering)

set(project_root ${CMAKE_SOURCE_DIR})

include_directories(
    ${project_root}/src/
    ${project_root}/vendor/SDL/include
    ${project_root}/vendor/sse2neon/
    ${project_root}/vendor/thread-pool/include
)

add_executable(${PROJECT_NAME} 
    ${project_root}/src/color.cpp
    ${project_root}/src/edge_function.cpp
    ${project_root}/src/line.cpp
    ${project_root}/src/mesh.cpp
    ${project_root}/src/point.cpp
    ${project_root}/src/rect.cpp
    ${project_root}/src/texture.cpp
    ${project_root}/src/triangle.cpp
    ${project_root}/src/uv.cpp
    ${project_root}/src/main.cpp
    # SIMD
    ${project_root}/src/SIMD/vec4f32.cpp
    ${project_root}/src/SIMD/vec4i32.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)