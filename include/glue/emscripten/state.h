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

      glue::Value run(const std::string &code) const;

      template <class T> T get(const std::string &code) const { return run(code)->get<T>(); }

      MapValue root() const;
      void addModule(const MapValue &map);
    };

  }  // namespace emscripten
}  // namespace glue
