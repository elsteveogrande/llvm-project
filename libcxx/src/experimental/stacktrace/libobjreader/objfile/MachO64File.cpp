#include "ObjectFile.h"

#include "../debug/DWARFDebugInfo.h"
#include "../objfile/SectionTable.h"
#include "../objfile/SymbolTable.h"
#include "../util/Bytes.h"

namespace llvm::objreader {

namespace macho {

using vm_prot_t = int;
using cpu_type_t = int32_t;
using cpu_subtype_t = int32_t;

struct mach_header_64 {
  uint32_t magic;
  cpu_type_t cputype;
  cpu_subtype_t cpusubtype;
  uint32_t filetype;
  uint32_t ncmds;
  uint32_t sizeofcmds;
  uint32_t flags;
  uint32_t reserved;
};

struct load_command {
  enum class cmd_t : uint32_t {
    LC_SYMTAB = 0x02,
    LC_UUID = 0x1b,
    LC_SEGMENT_64 = 0x19,
  };

  cmd_t cmd;
  uint32_t cmdsize;
};

struct segment_command_64 : load_command {
  char segname[16];
  uint64_t vmaddr;
  uint64_t vmsize;
  uint64_t fileoff;
  uint64_t filesize;
  vm_prot_t maxprot;
  vm_prot_t initprot;
  uint32_t nsects;
  uint32_t flags;
};

struct section_64 {
  char sectname[16];
  char segname[16];
  uint64_t addr;
  uint64_t size;
  uint32_t offset;
  uint32_t align;
  uint32_t reloff;
  uint32_t nreloc;
  uint32_t flags;
  uint32_t reserved1;
  uint32_t reserved2;
  uint32_t reserved3;
};

struct symtab_command : load_command {
  uint32_t symoff;
  uint32_t nsyms;
  uint32_t stroff;
  uint32_t strsize;
};

struct nlist_64 {
  // clang-format off
  enum class Type : uint8_t {
    // non-stabs types
    N_UNDF     = 0x00,
    N_ABS      = 0x02,
    N_SECT     = 0x0a,
    N_PBUD     = 0x0c,
    N_INDR     = 0x0e,

    // stabs types (see `mach-o/stab.h`)
    N_GSYM     = 0x20,
    N_FNAME    = 0x22,
    N_FUN      = 0x24,
    N_STSYM    = 0x26,
    N_LCSYM    = 0x28,
    N_BNSYM    = 0x2e,
    N_AST      = 0x32,
    N_OPT      = 0x3c,
    N_RSYM     = 0x40,
    N_SLINE    = 0x44,
    N_ENSYM    = 0x4e,
    N_SSYM     = 0x60,
    N_SO       = 0x64,
    N_OSO      = 0x66,
    N_LSYM     = 0x80,
    N_BINCL    = 0x82,
    N_SOL      = 0x84,
    N_PARAMS   = 0x86,
    N_VERSION  = 0x88,
    N_OLEVEL   = 0x8A,
    N_PSYM     = 0xa0,
    N_EINCL    = 0xa2,
    N_ENTRY    = 0xa4,
    N_LBRAC    = 0xc0,
    N_EXCL     = 0xc2,
    N_RBRAC    = 0xe0,
    N_BCOMM    = 0xe2,
    N_ECOMM    = 0xe4,
    N_ECOML    = 0xe8,
    N_LENG     = 0xfe,
    N_PC       = 0x30,
  };
  // clang-format on

  union {
    uint32_t n_strx;
  } n_un;
  Type n_type;
  uint8_t n_sect; // 1-based index of section IFF hasSection(); else 0
  uint16_t n_desc;
  uint64_t n_value;

  Type type() { return Type(uint8_t(n_type) & 0xee); }
  bool isStab() { return uint8_t(n_type) & 0xe0; }
  bool isPExt() { return uint8_t(n_type) & 0x10; }
  bool isExt() { return uint8_t(n_type) & 0x01; }
};

} // namespace macho

using command = macho::load_command;
using cmd_type = command::cmd_t;
using header = macho::mach_header_64;
using segment = macho::segment_command_64;
using section = macho::section_64;
using symtab = macho::symtab_command;
using symbol = macho::nlist_64;

MachO64File::MachO64File(ObjectReader &reader, std::string_view path, int fd,
                         void *mmap, size_t size)
    : ObjectFile(reader, path, fd, mmap, size) {}

Bytes MachO64File::header() { return slice(0, 32); }

void eachLoadCommand(Bytes cur, std::function<void(Bytes, command *)> cb) {
  header *header;
  cur = cur.get(&header);
  cur = cur.truncate(header->sizeofcmds);
  while (cur) {
    command *cmd;
    cur.get(&cmd); // don't advance
    cb(cur.truncate(cmd->cmdsize), cmd);
    cur = cur.slice(cmd->cmdsize);
  }
}

void eachSegment(Bytes here, std::function<void(Bytes, segment *)> cb) {
  eachLoadCommand(here, [cb](Bytes cur, command *cmd) {
    if (cmd->cmd == cmd_type::LC_SEGMENT_64) {
      cb(cur, (segment *)cmd);
    }
  });
}

void eachSection(Bytes here, std::function<void(section *)> cb) {
  eachSegment(here, [cb](Bytes cur, segment *seg) {
    cur = cur.slice(72); // past segment header
    while (cur) {
      section *sec;
      cur = cur.get(&sec);
      cb(sec);
    }
  });
}

Memory::Unique<SectionTable> MachO64File::genSectionTable() {
  auto ret = reader_.mem_.makeUnique<SectionTable>(reader_);
  eachSection(*this, [&ret](macho::section_64 *sec) {
    size_t size = 16;
    while (size > 0 && !sec->sectname[size - 1]) {
      --size;
    }
    std::string_view name{sec->sectname, size};
    ret->add({name, sec->addr, sec->size, sec->offset, sec->size});
  });
  return ret;
}

Memory::Unique<SymbolTable> MachO64File::genSymbolTable() {
  auto &secTable = sectionTable();
  auto ret = reader_.mem_.makeUnique<SymbolTable>(reader_);
  eachLoadCommand(*this, [&, this](Bytes cur, command *cmd) {
    if (cmd->cmd == cmd_type::LC_SYMTAB) {
      auto *st = (symtab *)cmd;
      auto syms = slice(st->symoff, st->nsyms * sizeof(symbol));
      auto strs = slice(st->stroff, st->strsize);
      while (syms) {
        symbol *sym;
        syms = syms.get(&sym);
        auto name = strs.str(sym->n_un.n_strx);
        auto addr = sym->n_value;
        Section *sec;
        if (sym->n_sect) {
          sec = secTable.atIndex(sym->n_sect);
        }
        if (addr && (!name.empty())) {
          switch (sym->type()) {
          case symbol::Type::N_FUN:
          case symbol::Type::N_SECT:
          case symbol::Type::N_INDR: // TODO: I'm confused by wording on INDR
            ret->add({addr, name, sec});
            break;
          default: // ignore
          }
        }
      }
    }
  });
  return ret;
}

Memory::Unique<DebugInfo> MachO64File::genDebugInfo() {
  return reader_.mem_.makeUnique<DWARFDebugInfo>(reader_, *this);
}

} // namespace llvm::objreader
