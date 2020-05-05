#include <easy_iterator.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <glue/context.h>
#include <glue/emscripten/state.h>
#include <glue/instance.h>

#include <exception>
#include <unordered_map>
// TODO: remove
#include <iostream>

namespace glue {
  namespace emscripten {
    namespace detail {

      using Value = ::emscripten::val;
      using ObjectCache = std::unordered_map<const glue::Map *, Value>;

      Any jsToAny(const Value &);
      Value anyToJS(const Any &, ObjectCache *cache = nullptr);

      Value invokeAnyFunction(const AnyFunction &f, Value args) {
        glue::AnyArguments anyArgs;
        auto N = args["length"].as<size_t>();
        for (size_t i = 0; i < N; ++i) {
          anyArgs.push_back(jsToAny(args[i]));
        }
        return anyToJS(f.call(anyArgs));
      }
    }  // namespace detail
  }    // namespace emscripten
}  // namespace glue

EMSCRIPTEN_BINDINGS(glue_bindings) {
  using namespace emscripten;
  class_<glue::Any>("Any");
  class_<glue::Instance>("Instance");
  class_<glue::AnyFunction>("AnyFunction");
  class_<glue::Context>("Context");

  function("invokeAnyFunction", &glue::emscripten::detail::invokeAnyFunction);
  constant("extendsKey", std::string(glue::keys::extendsKey));
  constant("constructorKey", std::string(glue::keys::constructorKey));
  constant("__glueContext", glue::Context());

  // clang-format off
  EM_ASM(
    Module.__glueScriptRunner = function(code) {
      try {
        return eval(code);
      } catch (e) {
        return {__glue_error: e.toString()};
      }
    };

    Module.__glueInvoker = function(callback, arguments) {
      return callback(...arguments);
    };

    Module.__glueFunctionWrapper = function(callback) {
      const invoker = Module.invokeAnyFunction;
      return function(...arguments) {
        return invoker(callback, arguments);
      }
    };

    Module.__glueIsAny = function(val) {
      return val instanceof Module.Any;
    };

    Module.__glueCreateClass = function(members) {
      const constructor = members[Module.constructorKey];
      const C = function(...args) {
        if (args[0] === "__glue_use_value") {
          this.__glue_instance = args[1];
        } else {
          this.__glue_instance = constructor(...args);
        }
      };
      if (members[Module.extendsKey]) {
        C.prototype = Object.create(members[Module.extendsKey].prototype);
      } else {
        C.prototype = {
          delete: function() { this.__glue_instance.delete(); }
        }
      }
      for (const key in members) {
        const member = members[key];
        C.prototype[key] = function(...args){ 
          return member(this.__glue_instance, ...args);
        };
      }
      members['__constructWithValue'] = function(value){
        return new C("__glue_use_value" ,value);
      };
      return C;
    };

    Module.__glueGlobal = this;
  ,);
  // clang-format on
  }

  namespace glue {
    namespace emscripten {
      namespace detail {

        auto getScriptRunner() { return Value::module_property("__glueScriptRunner"); }
        auto getFunctionInvoker() { return Value::module_property("__glueInvoker"); }
        auto getFunctionWrapper() { return Value::module_property("__glueFunctionWrapper"); }
        auto getIsAnyCheck() { return Value::module_property("__glueIsAny"); }
        auto getGlobalObject() { return Value::module_property("__glueGlobal"); }
        auto getCreateClass() { return Value::module_property("__glueCreateClass"); }
        auto &getContext() { return Value::module_property("__glueContext").as<Context &>(); }

        struct JSFunction {
          Value data;
          Value invoker;

          Any operator()(const AnyArguments &args) const {
            std::vector<::emscripten::val> vArgs;
            for (auto &&arg : args) {
              vArgs.push_back(anyToJS(arg));
            }
            return jsToAny(invoker(data, Value::array(vArgs)));
          }
        };

        struct JSMap final : public revisited::DerivedVisitable<JSMap, glue::Map> {
          Value data;

          JSMap(Value d) : data(std::move(d)) {}

          Any get(const std::string &key) const { return jsToAny(data[key]); }

          void set(const std::string &key, const Any &value) { data.set(key, anyToJS(value)); }

          bool forEach(
              const std::function<bool(const std::string &, const Any &)> &callback) const {
            Value keys = Value::global("Object")["keys"](data);
            size_t i = 0;
            while (true) {
              auto key = keys[i];
              if (key.isUndefined()) {
                break;
              } else {
                callback(key.as<std::string>(), jsToAny(data[key]));
              }
              ++i;
            }
            return false;
          }
        };

        Any jsToAny(const Value &val) {
          std::string type = val.typeOf().as<std::string>();

          if (type == "undefined") {
            return Any();
          } else if (type == "number") {
            return val.as<double>();
          } else if (type == "string") {
            return val.as<std::string>();
          } else if (type == "boolean") {
            return val.as<bool>();
          } else if (type == "function") {
            using namespace revisited;
            using FunctionVisitable = DataVisitableWithBasesAndConversions<JSFunction, TypeList<>,
                                                                           TypeList<AnyFunction>>;
            Any v;
            v.set<FunctionVisitable>(JSFunction{val, getFunctionInvoker()});
            return v;
          } else if (type == "object") {
            if (val["__glue_error"].as<bool>()) {
              throw std::runtime_error(val["__glue_error"].as<std::string>());
            } else if (val["__glue_instance"].as<bool>()) {
              return jsToAny(val["__glue_instance"]);
            } else if (getIsAnyCheck()(val).as<bool>()) {
              return val.as<Any>();
            } else {
              return Any(JSMap{val});
            }
          } else {
            return Any(val);
          }
        }

        struct AnyToJSVisitor
            : revisited::RecursiveVisitor<const int &, const size_t &, const float &, double, bool,
                                          const std::string &, std::string, AnyFunction,
                                          const glue::Map &, const JSMap &, const JSFunction &,
                                          Value> {
          std::optional<Value> result;
          ObjectCache *cache = nullptr;

          AnyToJSVisitor(ObjectCache *c = nullptr) : cache(c) {}

          bool visit(Value v) override {
            result = std::move(v);
            return true;
          }

          bool visit(const int &v) override {
            result = Value(v);
            return true;
          }

          bool visit(const float &v) override {
            result = Value(v);
            return true;
          }

          bool visit(const size_t &v) override {
            result = Value(v);
            return true;
          }

          bool visit(double v) override {
            result = Value(v);
            return true;
          }

          bool visit(bool v) override {
            result = Value(v);
            return true;
          }

          bool visit(std::string v) override {
            result = Value(std::move(v));
            return true;
          }

          bool visit(const std::string &v) override {
            result = Value(v);
            return true;
          }

          bool visit(const JSFunction &v) override {
            result = v.data;
            return true;
          }

          bool visit(AnyFunction v) override {
            result = getFunctionWrapper()(std::move(v));
            return true;
          }

          bool visit(const JSMap &v) override {
            result = v.data;
            return true;
          }

          bool visit(const glue::Map &v) override {
            if (auto it = cache ? easy_iterator::find(*cache, &v) : nullptr) {
              result = it->second;
              return true;
            }
            Value object = Value::object();
            v.forEach([&](auto &&k, auto &&o) {
              object.set(k, anyToJS(o, cache));
              return false;
            });
            if (v.get(keys::classKey)) {
              detail::getContext().addRootMap(glue::MapValue(std::make_shared<JSMap>(object)));
              result = getCreateClass()(object);
            } else {
              result = object;
            }
            if (cache) {
              cache->emplace(std::make_pair(&v, *result));
            }
            return true;
          }
        };

        Value anyToJS(const Any &any, ObjectCache *cache) {
          if (!any) {
            return Value::undefined();
          }
          AnyToJSVisitor visitor(cache);
          if (any.accept(visitor)) {
            assert(visitor.result);
            return *visitor.result;
          } else {
            if (auto instance = detail::getContext().createInstance(any)) {
              auto constructor = revisited::visitor_cast<JSMap &>(**instance.classMap)
                                     .data["__constructWithValue"];
              return constructor(Value(instance.data));
            } else {
              return Value(any);
            }
          }
        }

      }  // namespace detail
    }    // namespace emscripten
  }      // namespace glue

  using namespace glue::emscripten;

  struct State::Data {
    detail::Value runCallback;
    std::shared_ptr<detail::JSMap> globalObject;

    Data()
        : runCallback(detail::getScriptRunner()),
          globalObject(std::make_shared<detail::JSMap>(detail::getGlobalObject())) {}
  };

  State::State() : data(std::make_unique<State::Data>()) {}

  State::~State() {}

  glue::Value State::run(const std::string &code) const {
    return detail::jsToAny(data->runCallback(code));
  }

  glue::MapValue State::root() const { return glue::MapValue(data->globalObject); }

  void State::addModule(const MapValue &map) {
    detail::ObjectCache cache;
    auto convertedMap = Value(detail::jsToAny(detail::anyToJS(map.data, &cache))).asMap();
    assert(convertedMap);

    auto r = root();
    convertedMap.forEach([&](auto &&key, auto &&value) {
      r[key] = value;
      return false;
    });
  }