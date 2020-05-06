#pragma once

#include <glue/value.h>

#include <memory>
#include <string>

namespace glue {
  namespace emscripten {

    class State {
    private:
      struct Data;
      std::unique_ptr<Data> data;

    public:
      State();
      ~State();

      /**
       * Runs the expression and returns the result as a `Value`
       */
      glue::Value run(const std::string &code) const;

      /**
       * Runs the expression and returns the result as `T`
       */
      template <class T> T get(const std::string &code) const { return run(code)->get<T>(); }

      /**
       * returns the value map of the global table.
       */
      MapValue root() const;

      /**
       * returns the value map of the modules root table.
       */
      MapValue moduleRoot() const;

      /**
       * Adds a module to the Module table.
       */
      void addModule(const MapValue &map) const { addModule(map, moduleRoot()); }

      /**
       * Adds a module to the target table.
       */
      void addModule(const MapValue &map, const MapValue &root) const;

      /**
       * returns a function to delete bound JS values
       */
      Value getValueDeleter() const;
    };

  }  // namespace emscripten
}  // namespace glue
