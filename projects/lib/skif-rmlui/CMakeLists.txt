cmake_minimum_required(VERSION 3.30)

project(SkifRmlUi VERSION 1.0 LANGUAGES C CXX)

find_package(GLFW)
find_package(RmlUi)
find_package(OpenGL)
find_package(Glad)

glad_add_library(glad_gl_core_33 STATIC EXCLUDE_FROM_ALL HEADER_ONLY REPRODUCIBLE QUIET MX LANGUAGE cpp API gl:core=3.3 EXTENSIONS NONE)
target_compile_definitions(glad_gl_core_33 PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(glad_gl_core_33 PUBLIC OpenGL::GL)

set(target skif-rmlui)
add_library(${target})
add_library(skif::rmlui::app ALIAS ${target})
target_include_directories(${target}
  PUBLIC
    include
  PRIVATE
    private)

target_compile_features(${target} PUBLIC cxx_std_20)
target_link_libraries(${target} PRIVATE RmlUi::RmlUi RmlUi::Debugger RmlUi::renderer::gl33 glfw glad_gl_core_33)

target_sources(${target}
  PRIVATE
    src/app.cpp
    src/RmlUi/RendererGlad33.cpp
    src/implementation/window_impl.cpp
    src/implementation/window_manager_impl.cpp
    src/implementation/event_loop_impl.cpp
    src/implementation/plugin_manager_impl.cpp)
