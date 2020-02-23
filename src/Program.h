#pragma once

#include "Options.h"

#include <vector>

namespace MetaMovie {
class Program final {
 public:
  Program();
  OptionAdapterVector& getOptionAdapters();
  void run();

 private:
  std::string inputPath;
  std::string outputPath;
  OptionAdapterVector options;
};
}