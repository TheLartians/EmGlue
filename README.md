[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Build/badge.svg)](https://github.com/TheLartians/EmGlue/actions)
[![Actions Status](https://github.com/TheLartians/EmGlue/workflows/Style/badge.svg)](https://github.com/TheLartians/EmGlue/actions)

# Glue bindings for JavaScript using Embind

JavaScript (Wasm) bindings for [Glue](https://github.com/TheLartians/Glue) using Emscripten.

## Usage

### Example

Using EmGlue you can interact with JavaScript using a simple binding interface.
The following example illustrates the basic usage.

```cpp
#include <glue/emscripten/state.h>
#include <glue/class.h>
#include <iostream>

void exampleBasics() {
  glue::emscripten::State state;

  // run JS code
  state.run("console.log('Hello JavaScript!')");

  // extract values
  std::cout << state.run("'Hello' + ' ' + 'C++!'")->get<std::string>() << std::endl;

  // extract maps
  auto map = state.run("({a: 1, b: '2'})").asMap();
  map["a"]->get<int>(); // -> 1

  // extract functions
  auto f = state.run("(function(a,b){ return a+b })").asFunction();
  f(3, 4).get<int>(); // -> 7

  // inject values
  auto global = state.root();
  global["x"] = 42;
  global["square"] = [](double x){ return x*x; };
  
  // interact with JS directly
  state.run("console.log(square(x))");
  // or using Glue
  global["console"]["log"](global["square"](global["x"]));
}
```

Classes and inheritance are also supported.

```cpp
struct A {
  std::string member;
  A(std::string m): member(m) {}
  auto method() const { return "member: " + member; }
};

void exampleModules() {
  glue::emscripten::State state;

  // standard glue module definitions are supported
  auto module = glue::createAnyMap();
  
  module["A"] = glue::createClass<A>()
    .addConstructor<std::string>()
    .addMember("member", &A::member)
    .addMethod("method", &A::method)
  ;

  // inject C++ classes and APIs into JavaScript
  state.addModule(module, state.root());

  state.run("a = new A('test');");
  state.run("console.log(a.member());");
  state.run("console.log(a.method());");
  
  // there are destructors in JavaScript 
  // -> be sure to delete your instances after use!
  state.run("a.delete(); a=undefined;");
}
```

Check the [API](include/glue/emscripten/state.h) and [tests](test/source/state.cpp) for functionality and examples.
See [here](https://github.com/TheLartians/TypeScriptXX) for a full example project using automatic TypeScript declarations.

### Use in your project

EmGlue can be easily integrated through [CPM.cmake](https://github.com/TheLartians/CPM.cmake).
If not available before, this will automatically add the Glue target as well.

```cmake
CPMAddPackage(
  NAME EmGlue
  VERSION 0.2
  GITHUB_REPOSITORY TheLartians/EmGlue
)

target_link_libraries(myLibrary EmGlue)
```

### Build and run tests

First, [install and activate](https://emscripten.org/docs/getting_started/downloads.html) the emsdk.

```bash
emcmake cmake -Htest -Bbuild
cmake --build build -j8
node build/EmGlueTests.js
```
