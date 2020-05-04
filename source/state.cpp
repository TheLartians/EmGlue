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

EMSCRIPTEN_BINDINGS(glue_bindings) {
  EM_ASM(
  __glueScriptRunner = function(code) {
    try {
      return eval(code);
    } catch (e) {
      return {__glue_error: e.toString()};
    }
  };

  __glueInvoker = function(callback, arguments) {
    return callback(...arguments);
  };

  __glueGlobal = this;
,);

  using namespace emscripten;

  class_<glue::AnyArguments>("AnyArguments");

  class_<glue::AnyFunction>("AnyFunction").function("call", &glue::AnyFunction::call);
}

namespace glue {
  namespace emscripten {
    namespace detail {

      using Value = ::emscripten::val;

      Any jsToAny(const Value &);
      Value anyToJS(const Any &);

      auto getScriptRunner() { return Value::global("__glueScriptRunner"); }

      auto getFunctionInvoker() { return Value::global("__glueInvoker"); }

      auto getGlobalObject() { return Value::global("__glueGlobal"); }

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

        bool forEach(const std::function<bool(const std::string &, const Any &)> &callback) const {
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
          return AnyFunction(JSFunction{val, getFunctionInvoker()});
        } else if (type == "object") {
          if (val["__glue_error"].as<bool>()) {
            throw std::runtime_error(val["__glue_error"].as<std::string>());
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
          result = Value(std::move(v));
          return true;
        }

        bool visit(const JSMap &v) override {
          result = v.data;
          return true;
        }

        bool visit(const glue::Map &) override {
          // TODO
          return false;
          // sol::table table(state, sol::create);
          // v.forEach([&](auto &&k, auto &&v) {
          //   table[k] = anyToSol(state, v, cache);
          //   return false;
          // });
          // if (cache) {
          //   (*cache)[&v] = table;
          // }
          // result = table;
          // return true;
        }
      };

      Value anyToJS(const Any &any) {
        if (!any) {
          return Value::undefined();
        }
        AnyToJSVisitor visitor;
        if (any.accept(visitor)) {
          assert(visitor.result);
          return *visitor.result;
        } else {
          // TODO: see if we can create a instance
          return Value::undefined();
          // return Value(any);
        }
      }

    }  // namespace detail
  }    // namespace emscripten
}  // namespace glue

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

void State::addModule(const MapValue &) {
  // TODO
}