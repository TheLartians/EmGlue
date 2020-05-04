[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Build/badge.svg)](https://github.com/TheLartians/EmGlue/actions)
[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Style/badge.svg)](https://github.com/TheLartians/EmGlue/actions)

# Glue bindings for JavaScript using Embind

This library is a work-in-progress.

## Build and run tests

```bash
emcmake cmake -Htest -Bbuild
cmake --build build -j8
node build/EmGlueTests.js
```
