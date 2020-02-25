#include "Program.h"

#include "Mp4File.h"

#include <iomanip>
#include <sstream>

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
class TimeAdapter : public OptionAdapter {
 public:
  TimeAdapter(std::tm& created, std::tm& modified)
      : created(created), modified(modified) {}
  bool canHandle(OptionIterator const& iterator) override {
    return iterator.get() == "-t" || iterator.get() == "-tc" ||
           iterator.get() == "-tm";
  }
  void accept(OptionIterator& iterator) override {
    std::string option(iterator.get());
    iterator.next();
    std::istringstream ss(iterator.get());
    std::tm t = {};
    ss >> std::get_time(&t, "%d.%m.%Y-%H:%M:%S");

    if (option == "-t" || option == "-tc") {
      created = t;
    }
    if (option == "-t" || option == "-tm") {
      modified = t;
    }

    iterator.next();
  }

 private:
  std::tm& created;
  std::tm& modified;
};
}  // namespace
Program::Program() {
  options.push_back(std::make_unique<InputAdapter>(inputPath));
  options.push_back(std::make_unique<OutputAdapter>(outputPath));
  options.push_back(std::make_unique<TimeAdapter>(createdTime, modifiedTime));
}

OptionAdapterVector& Program::getOptionAdapters() { return options; }

void Program::run() {
  Mp4File file(inputPath);
  auto metadata = file.getMetadata();
  metadata.created = mktime(&createdTime);
  metadata.modified = mktime(&modifiedTime);
  file.setMetadata(metadata);
}
}  // namespace MetaMovie