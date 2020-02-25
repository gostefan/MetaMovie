#pragma once

#include "Options.h"

#include <ctime>
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
  std::tm createdTime;
  std::tm modifiedTime;
  OptionAdapterVector options;
};
}