cmake_minimum_required(VERSION 3.30)

find_package(RmlUi REQUIRED)

set(target ${PROJECT_NAME})
add_executable(${target})
target_sources(${target} PRIVATE main.cpp)
target_link_libraries(${target} PRIVATE skif::rmlui::app RmlUi::RmlUi)

# Добавляем include директории для RmlUi (необходимы для плагинов)
target_include_directories(${target} PRIVATE
    $<TARGET_PROPERTY:skif::rmlui::app,INCLUDE_DIRECTORIES>
)

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Платформонезависимое копирование
add_custom_command(
  TARGET ${target} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target}>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${target}>/assets
  COMMENT "Copying assets to build directory"
)
