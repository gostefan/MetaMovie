#include "Program.h"

#include "Mp4File.h"


namespace MetaMovie {
namespace {
class InputAdapter : public OptionAdapter {
 public:
  InputAdapter(std::string& target) : target(target) {}
  bool canHandle(OptionIterator const& iterator) override {
    return iterator.get() == "-i";
  }
  void accept(OptionIterator& iterator) override {
    iterator.next();
    target = iterator.get();
    iterator.next();
  }

 private:
  std::string& target;
};
class OutputAdapter : public OptionAdapter {
 public:
  OutputAdapter(std::string& target) : target(target) {}
  bool canHandle(OptionIterator const& iterator) override {
    return iterator.get() == "-o";
  }
  void accept(OptionIterator& iterator) override {
    iterator.next();
    target = iterator.get();
    iterator.next();
  }

 private:
  std::string& target;
};
}  // namespace
Program::Program() {
  options.push_back(std::make_unique<InputAdapter>(inputPath));
  options.push_back(std::make_unique<OutputAdapter>(outputPath));
}

OptionAdapterVector& Program::getOptionAdapters()  {
  return options;
}

void Program::run() { Mp4File file(inputPath); }
}  // namespace MetaMovie