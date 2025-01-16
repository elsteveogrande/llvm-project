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

struct Bytes {
  // This byte range is: [base_, limit_)
  std::byte *base_{nullptr};  // starts here (i.e., inclusive)
  std::byte *limit_{nullptr}; // exclusive

  Bytes() = default;
  Bytes(Bytes const &) = default;
  Bytes &operator=(Bytes const &) = default;

  Bytes(std::byte *base, std::byte *limit) : base_(base), limit_(limit) {}
  Bytes(std::byte *base, size_t size) : base_(base), limit_(base + size) {}

  size_t size() const { return limit_ - base_; }

  operator bool() { return size(); }

  uint8_t u8(size_t i) {
    if (base_ >= (limit_ - i)) {
      throw std::range_error("");
    }
    return uint8_t(base_[i]);
  }
  uint16_t u16(size_t i) {
    return uint16_t(u8(i)) | uint16_t(uint16_t(u8(i + 1)) << 8);
  }
  uint32_t u32(size_t i) {
    return uint32_t(u16(i)) | uint32_t(uint32_t(u16(i + 2)) << 16);
  }
  uint64_t u64(size_t i) {
    return uint64_t(u32(i)) | uint64_t(uint64_t(u32(i + 4)) << 32);
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
    ret.limit_ = limit_;
    return ret;
  }

  Bytes slice(size_t off) {
    if (base_ > limit_ - off) {
      throw std::range_error("slice offset exceeds size");
    }
    return {base_ + off, limit_};
  }

  Bytes slice(size_t off, size_t new_size) {
    return slice(off).truncate(new_size);
  }

  template <class BR> BR as(size_t off, size_t new_size) {
    return slice(off, new_size).as<BR>();
  }

  Bytes truncate(size_t newSize) {
    auto *newLimit = base_ + newSize;
    return {base_, std::min(newLimit, limit_)};
  }

  template <class T> Bytes get(T **t) {
    auto tsize = sizeof(T);
    assert(size() >= tsize);        // enough?
    *t = (T *)(base_);              // write recasted ptr
    return {base_ + tsize, limit_}; // advanced forward
  }

  template <class T, class... TT> Bytes get(T **t, TT &&...tt) {
    auto next = get(t);
    return next.get(std::forward<TT>(tt)...);
  }

  template <class T> Bytes copy(T &t) {
    auto tsize = sizeof(T);
    assert(size() >= tsize);        // enough?
    t = *(T *)(base_);              // copy-assign from recasted ptr
    return {base_ + tsize, limit_}; // advanced forward
  }

  template <class T, class U = T> Bytes iget(T *dest) {
    *dest = (T)(*(U *)base_);
    return slice(sizeof(U));
  }

  template <class T> Bytes igetU32(T *dest) {
    *dest = (T)(*(uint32_t *)base_);
    return slice(4);
  }

  template <class T> Bytes igetU16(T *dest) {
    *dest = (T)(*(uint16_t *)base_);
    return slice(2);
  }

  template <typename Z> Bytes uleb(Z *dest) {
    auto ret = *this;
    uint64_t z{0};
    int r = 64;
    auto roll = [&]() {
      z = (z >> 7) | (z << 57);
      r -= 7;
    };
    while (true) {
      uint8_t byte;
      ret = ret.iget(&byte);
      z |= uint64_t(byte & 0x7f);
      roll();
      if (!(byte & 0x80)) {
        break;
      }
    }
    r %= 64;
    *dest = Z(z >> r);
    return ret;
  }

  template <typename Z> Bytes sleb(Z *dest) {
    auto ret = *this;
    uint64_t z{0};
    int r = 64;
    while (true) {
      uint8_t byte;
      ret = ret.iget(&byte);
      z >>= 7;
      z |= (uint64_t(byte) << 57);
      r -= 7;
      if (!(byte & 0x80)) {
        break;
      }
    }
    *dest = Z(z) >> r;
    return ret;
  }
};

} // namespace llvm::objreader
