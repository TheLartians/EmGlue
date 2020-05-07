#include <doctest/doctest.h>

// clang-format off
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

struct A {
  std::string member;
  A(std::string m): member(m) {}
  auto method() const { return "member: " + member; }
};

void exampleModules() {
  glue::emscripten::State state;

  // inject C++ classes and APIs into JavaScript
  auto module = glue::createAnyMap();
  
  module["A"] = glue::createClass<A>()
    .addConstructor<std::string>()
    .addMember("member", &A::member)
    .addMethod("method", &A::method)
  ;

  state.addModule(module, state.root());

  state.run("a = new A('test');");
  state.run("console.log(a.member());");
  state.run("console.log(a.method());");
  
  // there are no destructors in JavaScript 
  // -> be sure to delete your instances after use!
  state.run("a.delete(); a=undefined;");
}
// clang-format on

TEST_CASE("Example") {
  CHECK_NOTHROW(exampleBasics());
  CHECK_NOTHROW(exampleModules());
}
