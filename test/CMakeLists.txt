cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(EmGlueTests LANGUAGES CXX)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

cpmaddpackage("gh:onqtam/doctest@2.4.11")
cpmaddpackage("gh:TheLartians/Format.cmake@1.7.3")

if(TEST_INSTALLED_VERSION)
  find_package(EmGlue REQUIRED)
else()
  cpmaddpackage(NAME EmGlue SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Enable compiler warnings ----

target_compile_options(EmGlue PUBLIC -Wall -pedantic -Wextra -Werror
                                     -Wno-dollar-in-identifier-extension)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(EmGlueTests ${sources})
target_link_libraries(EmGlueTests doctest EmGlue)

set_target_properties(EmGlueTests PROPERTIES CXX_STANDARD 17)

# emscripten-specific flags

set(EMSCRIPTEN_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0")
set_target_properties(EmGlueTests PROPERTIES COMPILE_FLAGS ${EMSCRIPTEN_FLAGS})
target_link_libraries(EmGlueTests ${EMSCRIPTEN_FLAGS})

# ---- Add tests ----

enable_testing()
add_test(EmGlueTests node ${CMAKE_CURRENT_BINARY_DIR}/EmGlueTests.js)
