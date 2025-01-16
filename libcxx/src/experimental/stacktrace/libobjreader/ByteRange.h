#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <stdexcept>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

namespace llvm::objreader {

struct ByteRange {
  uint8_t *base_{nullptr};
  size_t size_{0};

  operator bool() { return base_ && size_; }

  uint8_t u8(size_t i) {
    if (i >= size_) {
      throw std::range_error("");
    }
    return base_[i];
  }
  uint16_t u16(size_t i) {
    return uint16_t(u8(i)) | uint16_t(uint16_t(u8(i + 1)) << 8);
  }
  uint32_t u32(size_t i) {
    return uint32_t(u16(i)) | uint32_t(uint32_t(u8(i + 2)) << 16);
  }
  uint64_t u64(size_t i) {
    return uint64_t(u32(i)) | uint64_t(uint64_t(u8(i + 4)) << 32);
  }

  int8_t i8(size_t i) { return int8_t(u8(i)); }
  int16_t i16(size_t i) { return int16_t(u16(i)); }
  int32_t i32(size_t i) { return int32_t(u32(i)); }
  int64_t i64(size_t i) { return int64_t(u64(i)); }

  std::string_view str(size_t off, size_t size) {
    auto *cstr = (char const *)(base_ + off);
    while (size > 0 && !cstr[size - 1]) {
      --size;
    }
    return {cstr, size};
  }

  std::string_view str(size_t off) {
    auto *cstr = (char const *)(base_ + off);
    return {cstr};
  }

  template <class BR> BR as() {
    BR ret;
    ret.base_ = base_;
    ret.size_ = size_;
    return ret;
  }

  ByteRange slice(size_t off) { return from(off).resize(size_ - off); }
  ByteRange slice(size_t off, size_t new_size) {
    return from(off).resize(new_size);
  }

  template <class BR> BR as(size_t off, size_t new_size) {
    return slice(off, new_size).as<BR>();
  }

  ByteRange resize(size_t new_size) {
    return {base_, (new_size < size_) ? new_size : size_};
  }

  ByteRange from(size_t off) { return {base_ + off, size_ - off}; }
};

} // namespace llvm::objreader
