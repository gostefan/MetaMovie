#include "Mp4File.h"

#include <array>
#include <cassert>
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
  virtual void parse(std::fstream& stream, uint32_t size) = 0;
};
using BlockHandlerPtr = std::unique_ptr<BlockHandler>;
class SkipBlockHandler : public BlockHandler {
  bool canHandle(std::string const& /*type*/) const override { return true; }
  void parse(std::fstream& /*stream*/, uint32_t /*size*/) override{};
};
class MvhdHeaderHandler : public BlockHandler {
 public:
  MvhdHeaderHandler(Mp4File::Metadata& metadata) : metadata(metadata) {}
  bool canHandle(std::string const& type) const override {
    return "mvhd" == type;
  }
  void parse(std::fstream& stream, uint32_t size) override {
    auto const pos = stream.tellg();
    auto const version = streamRead<uint8_t>(stream, Endian::LITTLE);
    auto const empty1 = streamRead<uint8_t>(stream, Endian::LITTLE);
    auto const empty2 = streamRead<uint16_t>(stream, Endian::LITTLE);
    auto const created = streamRead<uint32_t>(stream, Endian::BIG);
    metadata.created = static_cast<std::time_t>(created - 2082844800u);
    auto const modified = streamRead<uint32_t>(stream, Endian::BIG);
    metadata.modified = static_cast<std::time_t>(modified - 2082844800u);
    auto const timeScale = streamRead<uint32_t>(stream, Endian::BIG);
    auto const timeLength = streamRead<uint32_t>(stream, Endian::BIG);
    auto const playbackSpeed = streamRead<int32_t>(stream, Endian::BIG);
    auto const volume = streamRead<int16_t>(stream, Endian::BIG) / 256.;
    auto const empty3 = streamRead<int64_t>(stream, Endian::LITTLE) / 65536.;
    auto const empty4 = streamRead<int16_t>(stream, Endian::LITTLE) / 65536.;
    auto const geoMatrixA = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixB = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixU = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixC = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixD = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixV = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixX = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixY = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const geoMatrixW = streamRead<int32_t>(stream, Endian::BIG) / 65536.;
    auto const previewStart = streamRead<uint32_t>(stream, Endian::BIG);
    auto const previewLength = streamRead<uint32_t>(stream, Endian::BIG);
    auto const posterFrame = streamRead<uint32_t>(stream, Endian::BIG);
    auto const selectionStart = streamRead<uint32_t>(stream, Endian::BIG);
    auto const selectionLength = streamRead<uint32_t>(stream, Endian::BIG);
    auto const currentTime = streamRead<uint32_t>(stream, Endian::BIG);
    auto const nextTrackId = streamRead<uint32_t>(stream, Endian::BIG);
    assert(stream.tellg() == pos + static_cast<std::streampos>(size - 8));
  };

 private:
  Mp4File::Metadata& metadata;
};

template <size_t N>
BlockHandler& getHandler(std::array<BlockHandlerPtr, N>& handlers,
                         std::fstream& stream) {
  auto const blockType = std::string(
      streamRead<std::array<char, 4>>(stream, Endian::LITTLE).data(), 4);
  auto handlerIter = std::find_if(handlers.begin(), handlers.end(),
                                  [&blockType](BlockHandlerPtr const& handler) {
                                    return handler->canHandle(blockType);
                                  });
  if (handlerIter == handlers.end())
    throw std::exception("Unxpectedly no block handler available.");
  return **handlerIter;
}

class HeaderBlockHandler : public BlockHandler {
 public:
  HeaderBlockHandler(Mp4File::Metadata& metadata) : metadata(metadata) {}
  bool canHandle(std::string const& type) const override {
    return FILE_HEADER == type;
  }
  void parse(std::fstream& stream, uint32_t size) override {
    std::array<BlockHandlerPtr, 2> handlers{
        std::make_unique<MvhdHeaderHandler>(metadata),
        std::make_unique<SkipBlockHandler>()};

    auto const endPos = stream.tellg() + static_cast<std::streampos>(size - 8);
    while (stream.tellg() < endPos) {
      auto const position = stream.tellg();
      auto const blockSize = streamRead<uint32_t>(stream, Endian::BIG);
      auto& handler = getHandler(handlers, stream);
      handler.parse(stream, blockSize);

      if (stream.tellg() != position + static_cast<std::streampos>(blockSize))
        stream.seekg(position + static_cast<std::streampos>(blockSize));
    }
  }

 private:
  Mp4File::Metadata& metadata;
};

template <size_t N>
void parseBlocks(std::fstream& stream,
                 std::array<BlockHandlerPtr, N>& handlers) {
  while (stream.peek() != std::char_traits<char>::eof()) {
    auto const position = stream.tellg();
    auto const blockSize = streamRead<uint32_t>(stream, Endian::BIG);
    auto& handler = getHandler(handlers, stream);
    handler.parse(stream, blockSize);
    stream.seekg(position + static_cast<std::streampos>(blockSize));
  }
}
}  // namespace

Mp4File::Mp4File(std::string const& path) : path(path) {
  std::array<BlockHandlerPtr, 2> blockHandlers{
      std::make_unique<HeaderBlockHandler>(metadata),
      std::make_unique<SkipBlockHandler>()};

  ManagedFile file(path, std::ios_base::in | std::ios_base::binary);
  std::fstream& stream = file.get();
  parseBlocks(stream, blockHandlers);
  std::cout << "Parse successful." << std::endl;
}

Mp4File::Metadata Mp4File::getMetadata() { return metadata; }
void Mp4File::setMetadata(Metadata metadataIn) { metadata = metadataIn; }
}  // namespace MetaMovie