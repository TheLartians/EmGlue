#include <doctest/doctest.h>
#include <glue/class.h>
#include <glue/emscripten/state.h>
#include <glue/view.h>

TEST_CASE("State") {
  glue::emscripten::State state;

  CHECK_NOTHROW(state.run("console.assert(1==1)"));
  CHECK_THROWS(state.run("syntax(error]"));
  CHECK_THROWS(state.run("syntax(error]"));

  CHECK(state.get<int>("2+3") == 5);
  CHECK(state.get<std::string>("'a' + 'b'") == "ab");
  CHECK(state.get<bool>("true") == true);
  REQUIRE(state.run("({a: 1, b: 2})").asMap());
  CHECK(state.run("({a: 1, b: 2})").asMap()["a"]->get<int>() == 1);
  CHECK(state.run("({a: 1, b: 2})").asMap()["b"]->get<int>() == 2);
  CHECK(state.run("(function(a,b){ return a+b })").asFunction()(3, 4).get<int>() == 7);
}

TEST_CASE("Mapped Values") {
  glue::emscripten::State state;
  glue::MapValue root = state.root();

  SUBCASE("primitives") {
    CHECK_NOTHROW(root["x"] = 42);
    CHECK(root["x"]->get<int>() == 42);
    CHECK(state.get<int>("x") == 42);
    CHECK_NOTHROW(root["y"] = "x");
    CHECK(root["y"]->get<std::string>() == "x");
    CHECK(state.get<std::string>("y") == "x");
    CHECK_NOTHROW(root["z"] = true);
    CHECK(root["z"]->get<bool>() == true);
    CHECK_NOTHROW(root["x"] = size_t(128));
    CHECK(root["x"]->get<size_t>() == 128);
  }

  SUBCASE("callbacks") {
    CHECK_NOTHROW(root["f"] = [](int x) { return 42 + x; });
    auto view = glue::View(root);
    state.run("let args = new Module.AnyArguments(); f.call(args); args.delete();");

    REQUIRE(root["f"].asFunction());
    CHECK(root["f"].asFunction()(3).get<int>() == 45);
    CHECK(state.get<int>("f(10)") == 52);
    CHECK_NOTHROW(state.run("function g(a,b) return a+b end"));
    CHECK(root["g"].asFunction()(2, 3).get<int>() == 5);
  }

  SUBCASE("any") {
    static thread_local int counter = 0;
    struct A {
      int value;
      A(int v) : value(v) { counter++; }
      A(A &&o) : value(o.value) { counter++; }
      A(const A &) = delete;
      ~A() { counter--; }
    };
    CHECK_NOTHROW(root["a"] = A{43});
    CHECK(root["a"]->get<const A &>().value == 43);
    CHECK_NOTHROW(root["a"]->get<A &>().value = 3);
    CHECK(root["a"]->get<const A &>().value == 3);
    CHECK(counter == 1);
    root["a"] = glue::Any();
    CHECK(counter == 0);
  }

  SUBCASE("standard library") {
    CHECK(root["tostring"].asFunction()(42).get<std::string>() == "42");
  }

  SUBCASE("maps") {
    auto map = glue::createAnyMap();
    map["a"] = 44;
    map["b"] = [](int a, int b) { return a + b; };
    CHECK_NOTHROW(root["map"] = map);
    REQUIRE(root["map"].asMap());
    CHECK(root["map"].asMap().keys().size() == 2);
    CHECK(root["map"].asMap()["a"]->get<int>() == 44);
    CHECK(root["map"].asMap()["b"].asFunction()(26, 19).get<int>() == 45);
  }
}

TEST_CASE("Passthrough arguments") {
  glue::emscripten::State state;
  glue::MapValue root = state.root();

  root["f"] = [](glue::Any v) { return v; };
  CHECK(root["f"].asFunction()(46).get<int>() == 46);
  CHECK(root["f"].asFunction()("hello").get<std::string>() == "hello");
  CHECK_NOTHROW(state.run("assert(f(47) == 47)"));
  CHECK_NOTHROW(state.run("assert(f('test') == 'test')"));
  CHECK_NOTHROW(state.run("x = {a: 1,b: 2}; assert(f(x) == x)"));
  CHECK_NOTHROW(state.run("x = {1,2}; assert(f(x) == x)"));
  CHECK_NOTHROW(state.run("x = function(){}; assert(f(x) == x)"));
  CHECK_NOTHROW(state.run("x = nil; assert(f(x) == x)"));
}

TEST_CASE("Modules") {
  struct A {
    std::string member;
  };

  struct B : public A {
    B(std::string value) : A{value} {}
    int method(int v) { return int(member.size()) + v; }
  };

  auto module = glue::createAnyMap();
  auto inner = glue::createAnyMap();

  inner["A"] = glue::createClass<A>().addConstructor<>().addMember("member", &A::member);

  module["inner"] = inner;

  module["B"] = glue::createClass<B>(glue::WithBases<A>())
                    .addConstructor<std::string>()
                    .setExtends(inner["A"])
                    .addMethod("method", &B::method);

  module["createB"] = []() { return B("unnamed"); };

  glue::emscripten::State state;
  state.addModule(module);

  CHECK(state.run("local a = inner.A.__new(); a.setMember('testA'); return a.member()")
            ->as<std::string>()
        == "testA");
  CHECK(state.run("local b = B.__new('testB'); return b.member()")->as<std::string>() == "testB");
  CHECK(state.run("local b = createB(); return b.member()")->as<std::string>() == "unnamed");
}
