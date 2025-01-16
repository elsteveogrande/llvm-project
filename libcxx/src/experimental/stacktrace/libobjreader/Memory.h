#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>

namespace llvm::objreader {

/**
 * With the given caller-provided allocator, construct
 * a `Memory`, which will provide any needed allocators
 * derived (`rebind`ed) from that.
 *
 * Create one of these at the start of your `objreader`
 * session; this will ensure all allocations go through
 * the given allocator, with this `Memory` object "installing"
 * itself in a thread-local.
 */
class Memory {
  std::function<std::byte *(size_t)> allocBytes_;
  std::function<void(std::byte *, size_t)> deallocBytes_;

public:
  template <class A>
  explicit Memory(A &a)
      : Memory([&a](size_t size) mutable { return a.allocate(size); },
               [&a](std::byte *p, size_t n) mutable { a.deallocate(p, n); }) {}

  Memory(std::function<std::byte *(size_t)> allocBytes,
         std::function<void(std::byte *, size_t)> deallocBytes)
      : allocBytes_(allocBytes), deallocBytes_(deallocBytes) {}

  template <typename T> struct Alloc {
    using value_type = T;
    using pointer = T *;
    using const_pointer = T const *;
    template <class U> struct rebind {
      typedef Alloc<U> other;
    };

    Memory *memory_;

    T *allocate(size_t n) {
      assert(memory_);
      auto size = n * sizeof(T) + alignof(T) - 1;
      return (T *)memory_->allocBytes_(size);
    }

    void deallocate(T *ptr, size_t n) {
      assert(memory_);
      memory_->deallocBytes_((std::byte *)ptr, n);
    }
  };

#ifdef _WIN32
  using _C = wchar_t;
#else
  using _C = char;
#endif

  template <typename T> struct Unique {
    T *obj_{nullptr};

    Unique() = default;

    void clear() {
      if (obj_) {
        Alloc<T>().deallocate(obj_, 1);
        obj_ = nullptr;
      }
    }

    ~Unique() { clear(); }

    Unique(T *obj) : obj_(obj) {}

    T &operator*() {
      if (!obj_) {
        throw std::logic_error("object is null");
      }
      return *obj_;
    }

    T *operator->() { return *obj_; }

    operator bool() const { return obj_; }

    template <typename U = T> Unique(Unique<U> &&rhs) : obj_(rhs.obj_) {
      rhs.obj_ = nullptr;
    }

    template <typename U = T> Unique<T> &operator=(Unique<U> &&rhs) {
      obj_ = rhs.obj_;
      rhs.obj_ = nullptr;
      return *this;
    }
  };

  template <typename T, typename... A> Unique<T> makeUnique(A... args) {
    auto alloc = Alloc<T>();
    T *ret = new (alloc.allocate(1)) T(std::forward<A>(args)...);
    return Unique<T>(ret);
  }

  class String : public std::basic_string<_C, std::char_traits<_C>, Alloc<_C>> {
    using super = std::basic_string<_C, std::char_traits<_C>, Alloc<_C>>;

  public:
    template <typename... A>
    String(A &&...args) : super(std::forward<A...>(args)...) {}
    String(String const &) = delete;
    String &operator=(String const &) = delete;
  };

  template <typename T> class Vec : public std::vector<T, Alloc<T>> {
    using super = std::vector<T, Alloc<T>>;

  public:
    template <typename... A>
    Vec(A &&...args) : super(std::forward<A...>(args)...) {}
    Vec(Vec const &) = delete;
    Vec &operator=(Vec const &) = delete;
  };

  template <typename K, typename T>
  class Map
      : public std::map<K, T, std::less<K>, Alloc<std::pair<K const, T>>> {
    using super = std::map<K, T, std::less<K>, Alloc<std::pair<K const, T>>>;

  public:
    Map() : super() {}
    Map(Map const &) = delete;
    Map &operator=(Map const &) = delete;
  };
};

} // namespace llvm::objreader
