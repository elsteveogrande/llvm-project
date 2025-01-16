#include "../Memory.h"
#include "../ObjectReader.h"

#include <cassert>
#include <memory>

/*
 * TODO this should really be suite of unit tests;
 * for now this is at least handy for dev and debugging.
 */

int main(int argc, char **argv) {
  assert(argc > 1);

  std::allocator<std::byte> alloc;
  llvm::objreader::Memory memory{alloc};
  llvm::objreader::ObjectReader objReader{memory};

  for (int i = 1; i < argc; i++) {
    auto objFile = objReader.open(argv[i]);
  }
}
