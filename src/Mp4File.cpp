#include "Mp4File.h"

#include <array>
#include <fstream>
#include <vector>

namespace {
std::string const FILE_HEADER = "moov";

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

enum class Endian { BIG, LITTLE };
template <typename T>
T streamRead(std::fstream& stream, Endian endian) {
  size_t const size = sizeof(T) / sizeof(char);
  auto data = std::make_unique<char[]>(size);
  stream.read(data.get(), size);
  if (endian == Endian::BIG) std::reverse(data.get(), data.get() + size);
  T result;
  std::memcpy(&result, data.get(), size);
  return result;
}

class BlockHandler {
 public:
  virtual bool canHandle(std::string const& type) const = 0;
  virtual void handle(std::fstream& stream, uint32_t size) = 0;
};
using BlockHandlerPtr = std::unique_ptr<BlockHandler>;
class SkipBlockHandler : public BlockHandler {
  bool canHandle(std::string const& /*type*/) const override { return true; }
  void handle(std::fstream& /*stream*/, uint32_t /*size*/) override {};
};
class HeaderBlockHandler : public BlockHandler {
  bool canHandle(std::string const& type) const override {
    return FILE_HEADER == type;
  }
  void handle(std::fstream& /*stream*/, uint32_t /*size*/) override {
    // TODO: Read header
  };
};

template <size_t N>
BlockHandler& getHandler(std::array<BlockHandlerPtr, N>& handlers, std::fstream& stream) {
  auto const blockType = std::string(
      streamRead<std::array<char, 4>>(stream, Endian::LITTLE).data(), 4);
  auto handlerIter = std::find_if(handlers.begin(), handlers.end(),
                                  [blockType](BlockHandlerPtr const& handler) {
                                    return handler->canHandle(blockType);
                                  });
  if (handlerIter == handlers.end())
    throw std::exception("Unxpectedly no block handler available.");
  return **handlerIter;
}
template <size_t N>
void parseBlocks(std::fstream& stream, std::array<BlockHandlerPtr, N>& handlers) {
  while (stream.peek() != std::char_traits<char>::eof()) {
    auto const position = stream.tellg();
    auto const blockSize = streamRead<uint32_t>(stream, Endian::BIG);
    auto& handler = getHandler(handlers, stream);
    handler.handle(stream, blockSize);
    stream.seekg(position + static_cast<std::streampos>(blockSize));
  }
}
}  // namespace

namespace MetaMovie {
Mp4File::Mp4File(std::string const& path) : path(path) {
  std::array<BlockHandlerPtr, 2> blockHandlers{
      std::make_unique<HeaderBlockHandler>(),
      std::make_unique<SkipBlockHandler>()};

  ManagedFile file(path, std::ios_base::in | std::ios_base::binary);
  std::fstream& stream = file.get();
  parseBlocks(stream, blockHandlers);
}
}  // namespace MetaMovie