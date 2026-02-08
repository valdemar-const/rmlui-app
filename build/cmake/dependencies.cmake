if (USE_CPM AND CPM_DOWNLOAD)

include(CPM)

CPMAddPackage(NAME GLAD
  URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip
  VERSION 2.0.8
  DOWNLOAD_ONLY
  SOURCE_SUBDIR cmake
  EXCLUDE_FROM_ALL)

if (GLAD_ADDED)
  list(APPEND CMAKE_MODULE_PATH ${GLAD_SOURCE_DIR}/cmake)
endif()

CPMAddPackage(
  NAME GLFW
  URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
  VERSION 3.4
  DOWNLOAD_ONLY)

CPMAddPackage(
  NAME Freetype
  URL https://download.savannah.gnu.org/releases/freetype/freetype-2.14.1.tar.xz
  VERSION 2.14.1
  DOWNLOAD_ONLY
  OPTIONS
  "BUILD_SHARED_LIBS NO"
  "FT_DISABLE_BROTLI YES"
  "FT_DISABLE_HARFBUZZ YES"
  "FT_DISABLE_PNG YES"
  "FT_DISABLE_ZLIB YES"
  "FT_DISABLE_BZIP2 YES"
)

if (Freetype_ADDED)
  # Add alias target to make the target provided by CPM conform to the standard naming
  # brought by the CMake-bundled FindFreetype.cmake find module
  add_library(Freetype::Freetype ALIAS freetype)
endif()

CPMAddPackage(NAME RmlUi
  VERSION 6.2
  GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
  GIT_TAG 6.2
  EXCLUDE_FROM_ALL YES
  GIT_SHALLOW YES)

endif()
