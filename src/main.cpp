#include "Options.h"
#include "Program.h"

#include <exception>
#include <iostream>


using namespace MetaMovie;

int main(int const argc, char const* const argv[]) {
  try {
    Program program;
    OptionConverter options(argc, argv, program.getOptionAdapters());
    program.run();
  } catch (std::exception e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
