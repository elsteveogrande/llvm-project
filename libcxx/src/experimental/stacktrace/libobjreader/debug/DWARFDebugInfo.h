#pragma once

#include "DebugInfo.h"

#include "../ObjectReader.h"
#include "../objfile/ObjectFile.h"
#include "SourceLoc.h"

#include <stdexcept>

namespace llvm::objreader {

namespace dwarf5 {

// https://dwarfstd.org/doc/DWARF5.pdf

enum ContentType {
  // DW_LNCT_* constants
  INVALID = 0,
  path = 1,
  dirindex = 2,
  timestamp = 3,
  size = 4,
  md5 = 5,
};

inline ContentType contentType(unsigned x) {
  using T = ContentType;
  constexpr static T types[]{T::INVALID,   T::path, T::dirindex,
                             T::timestamp, T::size, T::md5};
  return (x <= sizeof(types)) ? types[x] : T::INVALID;
}

/** 7.5.6 Form Encodings */
enum class FormType : uint8_t {
  // DW_FORM_* constants
  INVALID = 0,
  addr = 0x01,
  // 0x02: reserved
  block2 = 0x03,
  block4 = 0x04,
  data2 = 0x05,
  data4 = 0x06,
  data8 = 0x07,
  string = 0x08,
  block = 0x09,
  block1 = 0x0a,
  data1 = 0x0b,
  flag = 0x0c,
  sdata = 0x0d,
  strp = 0x0e,
  udata = 0x0f,
  ref_addr = 0x10,
  ref1 = 0x11,
  ref2 = 0x12,
  ref4 = 0x13,
  ref8 = 0x14,
  ref_udata = 0x15,
  indirect = 0x16,
  sec_offset = 0x17,
  exprloc = 0x18,
  flag_present = 0x19,
  strx = 0x1a,
  addrx = 0x1b,
  ref_sup4 = 0x1c,
  strp_sup = 0x1d,
  data16 = 0x1e,
  line_strp = 0x1f,
  ref_sig8 = 0x20,
  implicit_const = 0x21,
  loclistx = 0x22,
  rnglistx = 0x23,
  ref_sup8 = 0x24,
  strx1 = 0x25,
  strx2 = 0x26,
  strx4 = 0x27,
  strx8 = 0x28,
  addrx1 = 0x29,
  addrx2 = 0x2a,
  addrx3 = 0x2b,
  addrx4 = 0x2c,
};

inline FormType formType(unsigned x) {
  using T = FormType;
  constexpr static T types[]{
      T::INVALID,  T::addr,           T::INVALID,  T::block2,
      T::block4,   T::data2,          T::data4,    T::data8,
      T::string,   T::block,          T::block1,   T::data1,
      T::flag,     T::sdata,          T::strp,     T::udata,
      T::ref_addr, T::ref1,           T::ref2,     T::ref4,
      T::ref8,     T::ref_udata,      T::indirect, T::sec_offset,
      T::exprloc,  T::flag_present,   T::strx,     T::addrx,
      T::ref_sup4, T::strp_sup,       T::data16,   T::line_strp,
      T::ref_sig8, T::implicit_const, T::loclistx, T::rnglistx,
      T::ref_sup8, T::strx1,          T::strx2,    T::strx4,
      T::strx8,    T::addrx1,         T::addrx2,   T::addrx3,
      T::addrx4,
  };
  return (x <= sizeof(types)) ? types[x] : T::INVALID;
}

struct EntryFormat final {
  ContentType contentType;
  FormType formType;
};

struct EntryData final {
  constexpr static size_t kMax = 4;
  int values{0};
  ContentType contentTypes[kMax]{ContentType::INVALID};
  FormType formTypes[kMax]{FormType::INVALID};
  uint64_t u64s[kMax]{0};
  std::string_view strs[kMax]{};

  static Bytes read(Bytes pos, FormType ftype, bool is64Bit, uint64_t &u64,
                    std::string_view &str, Bytes lineStrs) {
    uint64_t tmpU64;
    switch (ftype) {
    case FormType::line_strp:
      if (is64Bit) {
        pos = pos.iget(&tmpU64);
      } else {
        pos = pos.iget<uint64_t, uint32_t>(&tmpU64);
      }
      str = lineStrs.str(tmpU64);
      return pos;

    case FormType::udata:
      pos = pos.uleb(&u64);
      return pos;

    case FormType::addr:
    case FormType::block2:
    case FormType::block4:
    case FormType::data2:
    case FormType::data4:
    case FormType::data8:
    case FormType::string:
    case FormType::block:
    case FormType::block1:
    case FormType::data1:
    case FormType::flag:
    case FormType::sdata:
    case FormType::strp:
    case FormType::ref_addr:
    case FormType::ref1:
    case FormType::ref2:
    case FormType::ref4:
    case FormType::ref8:
    case FormType::ref_udata:
    case FormType::indirect:
    case FormType::sec_offset:
    case FormType::exprloc:
    case FormType::flag_present:
    case FormType::strx:
    case FormType::addrx:
    case FormType::ref_sup4:
    case FormType::strp_sup:
    case FormType::data16:
    case FormType::ref_sig8:
    case FormType::implicit_const:
    case FormType::loclistx:
    case FormType::rnglistx:
    case FormType::ref_sup8:
    case FormType::strx1:
    case FormType::strx2:
    case FormType::strx4:
    case FormType::strx8:
    case FormType::addrx1:
    case FormType::addrx2:
    case FormType::addrx3:
    case FormType::addrx4:
    default:
      // TODO graceful handling
      __builtin_unreachable();
    }
  }

  Bytes read(Bytes pos, ContentType ctype, FormType ftype, bool is64Bit,
             Bytes lineStrs) {
    if (values == kMax) {
      throw std::range_error("too many values in format");
    }
    pos = read(pos, ftype, is64Bit, u64s[values], strs[values], lineStrs);
    contentTypes[values] = ctype;
    formTypes[values] = ftype;
    ++values;
    return pos;
  }

  std::string_view pathStr() {
    for (size_t i = 0; i < kMax; i++) {
      if (contentTypes[i] == ContentType::path) {
        return strs[i];
      }
    }
    return {};
  }

  size_t dirIndex() {
    for (size_t i = 0; i < kMax; i++) {
      if (contentTypes[i] == ContentType::dirindex) {
        return u64s[i];
      }
    }
    return 0;
  }
};

/** 6.2.4 The Line Number Program Header */
struct Header final {
  // An "initial length" value; a 32-bit value, or 0xffffffff followed by a
  // 64-bit value.
  size_t length{}; // size of line info, after this field of 4 or 12 bytes
  bool is64bit{};
  uint16_t version{};
  uint8_t addrSize{};
  uint8_t segSelectorSize{};
  uint8_t minInsnLength{};
  uint8_t maxOpsPerInsn{};
  bool defaultIsStmt{};
  int8_t lineBase{};
  uint8_t lineRange{};
  uint8_t opcodeBase{};
  uint8_t stdOpcodeLens[32]; // defines indexes 1 to opcodeBase-1 (usually 14)
};

} // namespace dwarf5

struct DWARFDebugTable;
struct DWARF5DebugMachine;

struct DWARFDebugInfo final : DebugInfo {
  Memory::List<DWARFDebugTable> tables_;

  virtual ~DWARFDebugInfo() = default;
  DWARFDebugInfo(ObjectReader &reader, ObjectFile &objFile);
  SourceLoc sourceLocAtAddr(uintptr_t addr) override;
};

/** A pair of `Bytes` objects representing the contents of sections
".debug_line" and ".debug_line_strs" (leading underscores if MachO file) */
struct DWARFDebugTable final {
  ObjectReader &reader_;
  Bytes lineData_;
  Bytes lineStrs_;
  Memory::Map<uintptr_t, SourceLoc> locs_;
  Memory::Unique<DWARF5DebugMachine> machine_{};

  DWARFDebugTable(ObjectReader &reader, Bytes lineData, Bytes lineStrs);

  SourceLoc sourceLocAtAddr(uintptr_t addr);
};

/** Evaluate the `.debug_line data from available `DWARFDebugTable`'s via a
 * "state machine" as described in the DWARF 5 spec:
 * See Section 6.2, "Line Number Information"
 */
struct DWARF5DebugMachine final {
  Bytes lineData_;
  Bytes lineStrs_;
  Bytes nextUnit_;
  dwarf5::Header header_;
  Memory::Vec<dwarf5::EntryFormat> dirEntryFormats;
  Memory::Vec<dwarf5::EntryData> directories;
  Memory::Vec<dwarf5::EntryFormat> fileEntryFormats;
  Memory::Vec<dwarf5::EntryData> filenames;
  Bytes insn_;

  // See "6.2.2 State Machine Registers"
  std::byte *addr;
  int opIndex;
  unsigned file;
  unsigned line;
  unsigned column;
  bool isStmt;
  bool basicBlock;
  bool endSeq;
  bool prologEnd;
  bool epilogBegin;
  unsigned isa = 0;
  unsigned discrim = 0;

  DWARF5DebugMachine(ObjectReader &reader, Bytes lineData, Bytes lineStrs);

  /** Initialize / reset SM registers */
  void reset();

  /** Create SourceLoc based on the current state, write to destination */
  void emitLoc(SourceLoc &loc);

  void addrAndOpAdvance(uint64_t operAdvance);

  void doSpecialOp(uint8_t op);

  /** From the current state of this machine, continue interpreting line data,
   * stopping at the next source location.  Subsequent calls will continue from
   * after that point.  This is to avoid unnecessary computation of line info.
   * The caller (DWARFDebugTable) will request only up to a certain address and
   * stop requesting when it's found. */
  SourceLoc nextLoc();

  /** Keep interpreting instructions until the next location is found. */
  bool nextLoc(SourceLoc &ret);
};

} // namespace llvm::objreader
