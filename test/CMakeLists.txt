cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(EmGlueTests
  LANGUAGES CXX
)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY onqtam/doctest
  GIT_TAG 2.3.7
)

if (TEST_INSTALLED_VERSION)
  find_package(EmGlue REQUIRED)
else()
  CPMAddPackage(
    NAME EmGlue
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.3
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(EmGlueTests ${sources})
target_link_libraries(EmGlueTests doctest EmGlue)

set_target_properties(EmGlueTests PROPERTIES CXX_STANDARD 17)

# emscripten-specific flags

set(EMSCRIPTEN_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0")
set_target_properties(EmGlueTests PROPERTIES COMPILE_FLAGS ${EMSCRIPTEN_FLAGS})
target_link_libraries(EmGlueTests ${EMSCRIPTEN_FLAGS})

# enable compiler warnings
target_compile_options(EmGlue PUBLIC -Wall -pedantic -Wextra -Werror -Wno-dollar-in-identifier-extension)

