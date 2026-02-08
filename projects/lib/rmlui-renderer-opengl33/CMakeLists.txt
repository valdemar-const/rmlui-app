cmake_minimum_required(VERSION 4.0)

project(RmlUiRendererOpenGL33 VERSION 6.2 LANGUAGES C CXX)

find_package(RmlUi 6.2 REQUIRED)

set(target ${PROJECT_NAME})
add_library(${target} INTERFACE)
add_library(RmlUi::renderer::gl33 ALIAS ${target})

target_compile_definitions(${target} INTERFACE RML_RENDERER_OPENGL33_IMPLEMENTATION)
target_include_directories(${target}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${target} INTERFACE RmlUi::RmlUi)
