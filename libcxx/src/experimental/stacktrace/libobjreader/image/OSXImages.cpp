#include "Image.h"

#include "../ObjectReader.h"

#if __has_include(<mach-o/dyld.h>)
#include <mach-o/dyld.h>

namespace llvm::objreader {
OSXImages::OSXImages(ObjectReader &reader) : Images{reader} {
  auto n = _dyld_image_count();
  for (uint32_t i = 0; i < n; i++) {
    add({// filename; we'll try to match this to one in `openFiles_`
         _dyld_get_image_name(i),
         // module is loaded into this virtual addr
         uintptr_t(_dyld_get_image_header(i)),
         // for adjusting symbols' ASLR addr -> symtable, debuginfo etc. addrs
         _dyld_get_image_vmaddr_slide(i),
         // first entry is main program
         i == 0});
  }
}
} // namespace llvm::objreader

#else

namespace llvm::objreader {
OSXImages::OSXImages(Memory &mem) : Images{mem} {}
} // namespace llvm::objreader

#endif
