#include "Mp4File.h"

#include <fstream>
#include <iostream>

namespace {
class ManagedFile {
 public:
  ManagedFile(std::string const& path, std::ios_base::openmode mode)
      : stream(path.c_str(), mode) {}
  ~ManagedFile() noexcept {
    try {
      stream.close();
    } catch (...) {
    }
  }

  std::fstream& get() { return stream; }

 private:
  std::fstream stream;
};

template <typename T>
T streamRead(std::fstream& stream) {
  size_t const size = sizeof(T) / sizeof(char);
  auto data = std::make_unique<char[]>(size);
  stream.read(data.get(), size);
  std::reverse(data.get(), data.get() + size);
  T result;
  std::memcpy(&result, data.get(), size);
  return result;
}
}  // namespace

namespace MetaMovie {
Mp4File::Mp4File(std::string const& path) : path(path) {
  ManagedFile file(path, std::ios_base::in | std::ios_base::binary);
  std::fstream& stream = file.get();
  while (stream.peek() != std::char_traits<char>::eof()) {
    auto const blockSize = streamRead<uint32_t>(stream);
    stream.seekg(blockSize - sizeof(uint32_t), std::ios_base::cur);
  }
}
}  // namespace MetaMovie