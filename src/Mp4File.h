#pragma once

#include <chrono>
#include <string>


namespace MetaMovie {
class Mp4File final {
 public:
  Mp4File(std::string const& path);
 
 private:
  std::string path;
};
}  // namespace MetaMovie