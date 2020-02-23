#include "Options.h"

#include <string>
#include <vector>

namespace MetaMovie {
OptionIterator::OptionIterator(int const argc, char const* const argv[])
    : position(1) {
  for (int i = 0; i < argc; i++) {
    strings.emplace_back(argv[i]);
  }
}

bool OptionIterator::hasNext() const { return strings.size() > position; }
void OptionIterator::next() { position++; }
std::string const& OptionIterator::get() const { return strings[position]; }

OptionConverter::OptionConverter(int const argc, char const* const argv[],
                                 OptionAdapterVector& adapters) {
  OptionIterator iterator(argc, argv);
  while (iterator.hasNext()) {
    auto adapterIter =
        std::find_if(adapters.begin(), adapters.end(),
                     [&iterator](OptionAdapterPtr const& adapter) {
                       return adapter->canHandle(iterator);
                     });
    if (adapterIter == adapters.end()) {
      throw std::exception("Unknown option.");
    } else {
      auto& adapter = **adapterIter;
      adapter.accept(iterator);
    }
  }
}
}  // namespace MetaMovie