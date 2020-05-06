cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(EmGlue 
  VERSION 0.2
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME Glue
  GITHUB_REPOSITORY TheLartians/Glue
  VERSION 1.3.2
)

CPMAddPackage(
  NAME EasyIterator
  VERSION 1.4
  GITHUB_REPOSITORY TheLartians/EasyIterator
)

# ---- Add source files ----

FILE(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
FILE(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

add_library(EmGlue ${headers} ${sources})

set_target_properties(EmGlue PROPERTIES CXX_STANDARD 17)

target_compile_options(EmGlue PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")
target_link_options(EmGlue PUBLIC "--bind")

target_link_libraries(EmGlue PUBLIC Glue)

target_include_directories(EmGlue
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)
