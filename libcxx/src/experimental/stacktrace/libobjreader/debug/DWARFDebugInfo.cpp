#include "DWARFDebugInfo.h"

#include "../objfile/ObjectFile.h"
#include "../objfile/SectionTable.h"
#include "SourceLoc.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace llvm::objreader {

DWARFDebugInfo::DWARFDebugInfo(ObjectReader &reader, ObjectFile &objFile)
    : DebugInfo(reader, objFile), tables_(reader.mem_) {
  auto objFileStr = objFile.path();
  auto binPath = fs::path(objFileStr.begin(), objFileStr.end());
  auto dir = binPath.parent_path();
  auto filename = binPath.filename();

  // Files to search for debug info, starting with the current binary
  auto files = reader.mem_.makeList<fs::path>();
  files.push_back(binPath);

  // Try to find (for Mach-O type binaries), a companion debug file, like:
  // "foo.exe.dSYM/Contents/Resources/DWARF/foo.exe"
  if (dynamic_cast<MachO64File &>(objFile)) {
    auto filenameDSym = filename;
    filenameDSym.concat(".dSYM");
    auto dsymDwarfPath =
        dir / filenameDSym / "Contents" / "Resources" / "DWARF" / filename;
    if (fs::exists(dsymDwarfPath) && fs::is_regular_file(dsymDwarfPath)) {
      files.push_back(dsymDwarfPath);
    }
  }

  // ELF files may have their own debug info, and/or `.dwo` files.
  // In the latter case the ELF will have references to those.
  // **TODO**

  // Windows PE EXE's have references to `.pdb` files.
  // **TODO**

  // TODOs:
  // - ELF 64bit
  // - PE + PDBs
  // - verify UUIDs, ensure related files are actually... related

  for (auto &file : files) {
    auto obj = reader.open(file.string());
    if (!obj) {
      continue;
    }
    auto &secs = (*obj)->sectionTable();
    Section *lineDataSec;
    Section *lineStrsSec;
    if ((lineDataSec = secs.byName("__debug_line")) &&
        (lineStrsSec = secs.byName("__debug_line_str"))) {
      tables_.emplace_back(reader_, (*obj)->sectionContent(*lineDataSec),
                           (*obj)->sectionContent(*lineStrsSec));
    } else if ((lineDataSec = secs.byName(".debug_line")) &&
               (lineStrsSec = secs.byName(".debug_line_str"))) {
      tables_.emplace_back(reader_, (*obj)->sectionContent(*lineDataSec),
                           (*obj)->sectionContent(*lineStrsSec));
    }
  }
}

SourceLoc DWARFDebugInfo::sourceLocAtAddr(uintptr_t addr) {
  SourceLoc ret{};
  for (auto &table : tables_) {
    ret = table.sourceLocAtAddr(addr);
    if (ret) {
      break;
    }
  }
  return ret;
}

DWARFDebugTable::DWARFDebugTable(ObjectReader &reader, Bytes lineData,
                                 Bytes lineStrs)
    : reader_(reader), lineData_(lineData), lineStrs_(lineStrs),
      locs_(reader.mem_) {
  // add low sentinel (but not high sentinel)
  locs_.emplace(0, SourceLoc());
}

SourceLoc DWARFDebugTable::sourceLocAtAddr(uintptr_t addr) {
  while (machine_ || lineData_) {
    if (!machine_) {
      machine_ = reader_.mem_.makeUnique<DWARF5DebugMachine>(reader_, lineData_,
                                                             lineStrs_);
      lineData_ = machine_->nextUnit_;
    }
    while (machine_) {
      auto loc = machine_->nextLoc();
      if (loc) {
        locs_.emplace(loc.addr_, loc);
      } else {
        locs_.emplace(~0, SourceLoc()); // now add the high sentinel
        machine_.clear();
      }
    }
  }

  auto it = locs_.upper_bound(addr);
  --it;
  return it->second;
}

DWARF5DebugMachine::DWARF5DebugMachine(ObjectReader &reader, Bytes lineData,
                                       Bytes lineStrs)
    : lineData_(lineData), lineStrs_(lineStrs), dirEntryFormats(reader.mem_),
      directories(reader.mem_), fileEntryFormats(reader.mem_),
      filenames(reader.mem_) {
  auto cur = lineData.iget<size_t, uint32_t>(&header_.length);
  if (header_.length != 0xffffffffu) {
  } else {
    cur = cur.iget(&header_.length);
    header_.is64bit = true;
  }
  // This unit occupies `unit_length` bytes from this point (after the unit
  // length uleb itself); there are possibly units after this one.  One machine
  // is created per unit.  So indicate where the next machine would pick up.
  nextUnit_ = cur.slice(header_.length);
  cur = cur.truncate(header_.length);
  cur = cur.iget(&header_.version);
  cur = cur.iget(&header_.addrSize);
  cur = cur.iget(&header_.segSelectorSize);
  size_t headerLength;
  cur = (header_.is64bit) ? cur.iget(&headerLength)
                          : cur.iget<size_t, uint32_t>(&headerLength);
  auto programStart = cur.slice(headerLength);
  cur = cur.iget(&header_.minInsnLength);
  cur = cur.iget(&header_.maxOpsPerInsn);
  cur = cur.iget(&header_.defaultIsStmt);
  cur = cur.iget(&header_.lineBase);
  cur = cur.iget(&header_.lineRange);

  cur = cur.iget(&header_.opcodeBase);
  assert(header_.opcodeBase < sizeof(header_.stdOpcodeLens));
  for (int i = 1; i < header_.opcodeBase; i++) {
    cur = cur.iget(header_.stdOpcodeLens + i - 1);
  }

  uint8_t dirEntryFormatCount;
  cur = cur.iget(&dirEntryFormatCount);
  dirEntryFormats.reserve(dirEntryFormatCount);
  while (dirEntryFormatCount--) {
    unsigned ct, ft;
    cur = cur.uleb(&ct).uleb(&ft);
    dirEntryFormats.emplace_back(
        dwarf5::EntryFormat{dwarf5::contentType(ct), dwarf5::formType(ft)});
  }

  uint64_t dirCount;
  cur = cur.uleb(&dirCount);
  directories.reserve(dirCount);
  while (dirCount--) {
    dwarf5::EntryData dirData;
    for (auto &fmt : dirEntryFormats) {
      cur = dirData.read(cur, fmt.contentType, fmt.formType, header_.is64bit,
                         lineStrs);
      directories.emplace_back(std::move(dirData));
    }
  }

  uint8_t fileEntryFormatCount;
  cur = cur.iget(&fileEntryFormatCount);
  fileEntryFormats.reserve(fileEntryFormatCount);
  while (fileEntryFormatCount--) {
    unsigned ct, ft;
    cur = cur.uleb(&ct).uleb(&ft);
    fileEntryFormats.emplace_back(
        dwarf5::EntryFormat{dwarf5::contentType(ct), dwarf5::formType(ft)});
  }

  uint64_t fileCount;
  cur = cur.uleb(&fileCount);
  filenames.reserve(fileCount);
  while (fileCount--) {
    dwarf5::EntryData fileData;
    for (auto &fmt : fileEntryFormats) {
      cur = fileData.read(cur, fmt.contentType, fmt.formType, header_.is64bit,
                          lineStrs);
      filenames.emplace_back(std::move(fileData));
    }
  }

  if (cur.base_ != programStart.base_) {
    // TODO more gracefully handle where things aren't as expected
    return;
  }
  // Point to the first SM instruction
  insn_ = cur;
  // Finally, initialize SM state, and we're ready to process insns.
  reset();
}

void DWARF5DebugMachine::reset() {
  // Initialize state machine
  // See: "Table 6.4: Line number program initial state"
  addr = 0;
  opIndex = 0;
  file = 1;
  line = 1;
  column = 0;
  isStmt = header_.defaultIsStmt;
  basicBlock = false;
  endSeq = false;
  prologEnd = false;
  epilogBegin = false;
  isa = 0;
  discrim = 0;
}

SourceLoc DWARF5DebugMachine::nextLoc() {
  SourceLoc ret{};
  bool gotNext{false};
  while (!gotNext) {
    gotNext = nextLoc(ret);
  }
  return ret;
}

void DWARF5DebugMachine::emitLoc(SourceLoc &loc) {
  auto fileEntry = filenames[file];
  auto dirEntry = directories[fileEntry.dirIndex()];
  loc = SourceLoc{.addr_ = uintptr_t(addr),
                  .dir_ = dirEntry.pathStr(),
                  .file_ = fileEntry.pathStr(),
                  .line_ = line};
}

void DWARF5DebugMachine::addrAndOpAdvance(uint64_t operAdvance) {
  /*
   * As per 6.2.5.1 "Special Opcodes"; this is used by special opcodes but also
   * by standard opcode `DW_LNS_advance_pc`:
   *
   *   new address = address +
   *     minimum_instruction_length *
   *       ((op_index + operation advance) / maximum_operations_per_instruction)
   *
   *   new op_index =
   *     (op_index + operation advance) % maximum_operations_per_instruction
   */
  addr = addr + (header_.minInsnLength *
                 ((opIndex + operAdvance) / header_.maxOpsPerInsn));
  opIndex = (opIndex + operAdvance) % header_.maxOpsPerInsn;
}

bool DWARF5DebugMachine::nextLoc(SourceLoc &ret) {
  // TODO add enums for the three opcode kinds
  if (!insn_) {  // have we exhausted insn stream?
    return true; // if so, we emit the dummy SourceLoc already in ret; done
  }
  uint8_t op;                            // extended/standard/special opcode
  uint64_t utmp;                         // scratch variable for operand
  int64_t stmp;                          // scratch variable for operand
  insn_ = insn_.iget(&op);               // read first byte to detect type
  if (op == 0) {                         // 6.2.5.3 Extended Opcodes
    insn_ = insn_.uleb(&utmp).iget(&op); // represented by: [00] [size] [extOp]
    switch (op) {                        // handle extended opcode:

    case 1:          // DW_LNE_end_sequence: (no operands)
      endSeq = true; // update state
      emitLoc(ret);  // send this out based on SM state
      reset();       // in case there's more following this
      return true;   // tell caller we wrote this last loc

    case 2:                               // DW_LNE_set_address: [addr32/64]
      insn_ = (header_.addrSize == 8)     // set new absolute address,
                  ? insn_.iget(&addr)     // either 64 bit
                  : insn_.igetU32(&addr); // else 32 bit.
      return false;                       // done (no loc was output)

    case 3:                         // DW_LNE_set_discriminator:
      insn_ = insn_.uleb(&discrim); // set to new value
      return false;                 // done (no loc was output)

    default:                   // unhandled extended opcode
      __builtin_unreachable(); // TODO gracefully handle
    }
  } else if (op < header_.opcodeBase) { // 6.2.5.2 Standard Opcodes
    switch (op) {                       // handle op (operands vary)

    case 1:                // DW_LNS_copy
      emitLoc(ret);        // send this out based on SM state
      discrim = 0;         // reset registers
      basicBlock = false;  //
      prologEnd = false;   //
      epilogBegin = false; //
      return true;         // tell caller we wrote this last loc

    case 2:                      // DW_LNS_advance_pc
      insn_ = insn_.uleb(&utmp); // one operand:
      addrAndOpAdvance(utmp);    // see 6.2.5.1
      return false;              // done (no loc was output)

    case 3:                      // DW_LNS_advance_line
      insn_ = insn_.sleb(&stmp); // one operand:
      line += stmp;              // delta for line number
      return false;              // done (no loc was output)

    case 4:                      // DW_LNS_set_file
      insn_ = insn_.uleb(&utmp); //
      file = utmp;               //
      return false;              // done (no loc was output)

    case 5:                      // DW_LNS_set_column
      insn_ = insn_.uleb(&utmp); //
      column = utmp;             //
      return false;              // done (no loc was output)

    case 6:             // DW_LNS_negate_stmt
      isStmt = !isStmt; // flip flop flag
      return false;     // done (no loc was output)

    case 7:              // DW_LNS_set_basic_block
      basicBlock = true; //
      return false;      // done (no loc was output)

    case 8:                          // DW_LNS_const_add_pc:
      addrAndOpAdvance(              // "It advances the address and
          (255 - header_.opcodeBase) // op_index registers by the increments
          / header_.lineRange);      // corresponding to special opcode 255"
      return false;                  // done (no loc was output)

    case 9:                         // DW_LNS_fixed_advance_pc
      insn_ = insn_.igetU16(&utmp); // single "uhalf" unencoded opd
      addr += utmp;                 //
      return false;                 // done (no loc was output)

    case 10:            // DW_LNS_set_prologue_end
      prologEnd = true; // no opds; just set this `true`
      return false;     // done (no loc was output)

    case 11:              // DW_LNS_set_epilogue_begin
      epilogBegin = true; // no opds; just set this `true`
      return false;       // done (no loc was output)

    case 12:                     // DW_LNS_set_isa
      insn_ = insn_.uleb(&utmp); // single ULEB opd
      isa = utmp;                //
      return false;              // done (no loc was output)

    default:                   // unhandled standard opcode
      __builtin_unreachable(); // TODO gracefully handle
    }
  } else {                                   // 6.2.5.1 Special Opcodes:
    uint8_t adjOp = op - header_.opcodeBase; // adjusted opcode
    auto lr = header_.lineRange;             //
    uint8_t opAdv = adjOp / lr;              // operation advance
    addrAndOpAdvance(opAdv);                 // adjust as per this section
    line += header_.lineBase + (adjOp % lr); // adjust line
    emitLoc(ret);                            // send this out based on SM state
    basicBlock = false;                      //
    prologEnd = false;                       //
    epilogBegin = false;                     //
    discrim = 0;                             //
    return true;                             // these always emit locations
  }
}

} // namespace llvm::objreader
