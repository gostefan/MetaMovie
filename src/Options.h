#pragma once

#include <memory>
#include <string>
#include <vector>

namespace MetaMovie {

class OptionIterator final {
 public:
  OptionIterator(int const argc, char const* const argv[]);
  bool hasNext() const;
  std::string const& get() const;
  void next();

 private:
  size_t position;
  std::vector<std::string> strings;
};

class OptionAdapter {
 public:
  virtual bool canHandle(OptionIterator const& iterator) = 0;
  virtual void accept(OptionIterator& iterator) = 0;
};
using OptionAdapterPtr = std::unique_ptr<OptionAdapter>;
using OptionAdapterVector = std::vector<OptionAdapterPtr>;

class OptionConverter final {
 public:
  OptionConverter(int const argc, char const* const argv[],
                  OptionAdapterVector& adapters);
};

}