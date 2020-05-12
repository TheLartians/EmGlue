#include <easy_iterator.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <glue/context.h>
#include <glue/emscripten/state.h>
#include <glue/instance.h>

#include <exception>
#include <unordered_map>

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
  constant("toStringKey", std::string(glue::keys::operators::tostring));
  constant("constructorKey", std::string(glue::keys::constructorKey));
  constant("__glueContext", glue::Context());

  // clang-format off

#if defined(GLUE_USE_ES6_CODE)

  EM_ASM(
    Module.__glueScriptRunner = function(code) {
      try {
        return eval(code);
      } catch (e) {
        return {__glue_error: String(e)};
      }
    };

    Module.__glueInvoker = function(callback, args) {
      return callback(...args);
    };

    Module.__glueFunctionWrapper = function(callback) {
      const invoker = Module.invokeAnyFunction;
      return function(...args) {
        return invoker(callback, args);
      }
    };

    Module.__glueIsAny = function(val) {
      return val instanceof Module.Any;
    };

    Module.__glueConstructCallback = () => {};
    Module.__setGlueConstructCallback = (f) => Module.__glueConstructCallback = f;

    Module.__glueCreateClass = function(members) {
      const constructor = members[Module.constructorKey];
      const C = function(...args) {
        if (args[0] === "__glue_use_value") {
          this.__glue_instance = args[1];
          Module.__glueConstructCallback(this.__glue_instance);
        } else {
          this.__glue_instance = constructor(...args);
        }
      };

      if (members[Module.extendsKey]) {
        C.prototype = Object.create(members[Module.extendsKey].prototype);
      } else {
        C.prototype = ({
          delete: function() { this.__glue_instance.delete(); }
        })
      }
  
      if (members[Module.toStringKey]) {
        const stringConverter = members[Module.toStringKey];
        C.prototype.toString = function(){ return stringConverter(this); };
      }

      for (const key in members) {
        const member = members[key];
        C.prototype[key] = function(...args){ 
          return member(this.__glue_instance, ...args);
        };
        C[key] = member;
      }
      members['__constructWithValue'] = function(value){
        return new C("__glue_use_value" ,value);
      };
      return C;
    };

    Module.__glueDeleter = function(v){ v.delete(); };
    Module.__glueGlobal = this;
    Module.__glueModule = Module;
  ,);

#else
/**
 * The content of the EM_ASM below is a backwards-compatible version of the readable code above.
 */

  EM_ASM(
function _instanceof(left, right) { if (right != null && typeof Symbol !== "undefined" && right[Symbol.hasInstance]) { return !!right[Symbol.hasInstance](left); } else { return left instanceof right; } }

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && Symbol.iterator in Object(iter)) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

Module.__glueScriptRunner = function (code) {
  try {
    return eval(code);
  } catch (e) {
    return {
      __glue_error: String(e)
    };
  }
};

Module.__glueInvoker = function (callback, args) {
  return callback.apply(void 0, _toConsumableArray(args));
};

Module.__glueFunctionWrapper = function (callback) {
  var invoker = Module.invokeAnyFunction;
  return function () {
    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    return invoker(callback, args);
  };
};

Module.__glueIsAny = function (val) {
  return _instanceof(val, Module.Any);
};

Module.__glueConstructCallback = function () {};

Module.__setGlueConstructCallback = function (f) {
  return Module.__glueConstructCallback = f;
};

Module.__glueCreateClass = function (members) {
  var constructor = members[Module.constructorKey];

  var C = function C() {
    if ((arguments.length <= 0 ? undefined : arguments[0]) === "__glue_use_value") {
      this.__glue_instance = arguments.length <= 1 ? undefined : arguments[1];
      Module.__glueConstructCallback(this.__glue_instance);
    } else {
      this.__glue_instance = constructor.apply(void 0, arguments);
    }
  };

  if (members[Module.extendsKey]) {
    C.prototype = Object.create(members[Module.extendsKey].prototype);
  } else {
    C.prototype = {
      delete: function _delete() {
        this.__glue_instance.delete();
      }
    };
  }

  if (members[Module.toStringKey]) {
    var stringConverter = members[Module.toStringKey];

    C.prototype.toString = function () {
      return stringConverter(this);
    };
  }

  var _loop = function _loop(key) {
    var member = members[key];

    C.prototype[key] = function () {
      for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
        args[_key2] = arguments[_key2];
      }

      return member.apply(void 0, [this.__glue_instance].concat(args));
    };

    C[key] = member;
  };

  for (var key in members) {
    _loop(key);
  }

  members['__constructWithValue'] = function (value) {
    return new C("__glue_use_value", value);
  };

  return C;
};

Module.__glueDeleter = function (v) {
  v.delete();
};

Module.__glueGlobal = this;
Module.__glueModule = Module;
  ,);
#endif

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
        auto getModuleObject() { return Value::module_property("__glueModule"); }
        auto getCreateClass() { return Value::module_property("__glueCreateClass"); }
        auto getConstructCallback() { return Value::module_property("__glueConstructCallback"); }
        auto getGlueDeleter() { return Value::module_property("__glueDeleter"); }
        auto &getContext() { return Value::module_property("__glueContext").as<Context &>(); }
        auto constructCallbackSetter() {
          return Value::module_property("__setGlueConstructCallback");
        }

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

          bool forEach(const std::function<bool(const std::string &)> &callback) const {
            Value keys = Value::global("Object")["keys"](data);
            size_t i = 0;
            while (true) {
              auto key = keys[i];
              if (key.isUndefined()) {
                break;
              } else {
                callback(key.as<std::string>());
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
            v.forEach([&](auto &&k) {
              object.set(k, anyToJS(v.get(k), cache));
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
              auto value = Value(any);
              detail::getConstructCallback()(value);
              return value;
            }
          }
        }

      }  // namespace detail
    }    // namespace emscripten
  }      // namespace glue

  using namespace glue::emscripten;

  struct State::Data {
    detail::Value runCallback;
    std::shared_ptr<detail::JSMap> globalObject, moduleObject;

    Data()
        : runCallback(detail::getScriptRunner()),
          globalObject(std::make_shared<detail::JSMap>(detail::getGlobalObject())),
          moduleObject(std::make_shared<detail::JSMap>(detail::getModuleObject())) {}
  };

  State::State() : data(std::make_unique<State::Data>()) {}

  State::~State() {}

  glue::Value State::run(const std::string &code) const {
    return detail::jsToAny(data->runCallback(code));
  }

  glue::MapValue State::root() const { return glue::MapValue(data->globalObject); }

  glue::MapValue State::moduleRoot() const { return glue::MapValue(data->moduleObject); }

  void State::addModule(const MapValue &map, const MapValue &r) const {
    detail::ObjectCache cache;
    auto convertedMap = Value(detail::jsToAny(detail::anyToJS(map.data, &cache))).asMap();
    assert(convertedMap);

    convertedMap.forEach([&](auto &&key, auto &&value) {
      r[key] = value;
      return false;
    });
  }

  glue::Value State::getValueDeleter() const { return detail::jsToAny(detail::getGlueDeleter()); }

  glue::Value State::getConstructCallbackSetter() const {
    return detail::jsToAny(detail::constructCallbackSetter());
  }
