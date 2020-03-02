#pragma once

#include <ctime>
#include <string>


namespace MetaMovie {
class Mp4File final {
 public:
  Mp4File(std::string const& path);

  struct Metadata {
    std::time_t created;
    std::time_t modified;
  };
  Metadata getMetadata();
  void setMetadata(Metadata metadata);

  void write();

 private:
  std::string path;
  Metadata metadata;
};
}  // namespace MetaMovie