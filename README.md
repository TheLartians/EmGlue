[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Build/badge.svg)](https://github.com/TheLartians/EmGlue/actions)
[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Style/badge.svg)](https://github.com/TheLartians/EmGlue/actions)

# Glue bindings for JavaScript using Embind

JavaScript (Emscripten) bindings for [Glue](https://github.com/TheLartians/Glue).

## Documentation

Yet to be written. 
Check the [API](include/glue/emscripten/state.h) and [tests](test/source/state.cpp) for functionality and examples.

## Usage

### Include in your project

EmGlue can be easily integrated through CPM.
If not available before, this will automatically add a Lua and Glue target as well.

```cmake
CPMAddPackage(
  NAME EmGlue
  VERSION 0.1
  GITHUB_REPOSITORY TheLartians/EmGlue
)

target_link_libraries(myLibrary EmGlue)
```

### Build and run tests

```bash
emcmake cmake -Htest -Bbuild
cmake --build build -j8
node build/EmGlueTests.js
```
