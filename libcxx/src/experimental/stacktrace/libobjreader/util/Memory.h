#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <stdexcept>
#include <vector>

namespace llvm::objreader {

/**
 * With the given caller-provided allocator, construct a `Memory` instance,
 * which will provide any needed allocators derived (`rebind`ed) from that.
 *
 * Create one of these at the start of your `objreader` session; this will
 * ensure all allocations go through the given allocator.
 */
class Memory {
  // This class only takes byte-level alloc / dealloc functions, stripped of
  // type information, to allow somewhat easier bridging of the caller's code
  // (without template arguments).

  std::function<std::byte *(size_t)> allocBytes_;
  std::function<void(std::byte *, size_t)> deallocBytes_;

public:
  /** A class modeling `Allocator` which wraps the caller-provided allocator,
   * and directs alloc / dealloc requests to the same.  Does not implement most
   * of the optional parts of `Allocator`. */
  template <typename T> struct Alloc {
    using value_type = T;
    using pointer = T *;
    using const_pointer = T const *;
    template <class U> struct rebind {
      typedef Alloc<U> other;
    };

    Memory &mem_;
    explicit Alloc(Memory &mem) : mem_(mem) {}

    template <typename U = T> Alloc(Alloc<U> const &rhs) : mem_(rhs.mem_) {}

    template <typename U = T> Alloc &operator=(Alloc<U> const &rhs) {
      mem_ = rhs.mem_;
      return this;
    }

    T *allocate(size_t n, void const * /*ignored*/) { return allocate(n); }

    T *allocate(size_t n) {
      auto size = n * sizeof(T) + alignof(T) - 1;
      return (T *)mem_.allocBytes_(size);
    }

    void deallocate(T *ptr, size_t n) {
      mem_.deallocBytes_((std::byte *)ptr, n);
    }
  };

  template <class A>
  explicit Memory(A &a)
      : Memory([&a](size_t size) mutable { return a.allocate(size); },
               [&a](std::byte *p, size_t n) mutable { a.deallocate(p, n); }) {}

  Memory(std::function<std::byte *(size_t)> allocBytes,
         std::function<void(std::byte *, size_t)> deallocBytes)
      : allocBytes_(allocBytes), deallocBytes_(deallocBytes) {}

  // Some convenience types which wrap common `std` classes with our allocator.

  /** Like a `unique_ptr`, but using our allocator */
  template <typename T> struct Unique final {
    T *obj_{nullptr};
    std::function<void()> deleter_{[] {}};

    void clear() {
      if (obj_) {
        deleter_();
        obj_ = nullptr;
        deleter_ = []() {};
      }
    }

    ~Unique() { clear(); }

    Unique() = default;
    Unique(T *obj, std::function<void()> deleter)
        : obj_(obj), deleter_(deleter) {}

    T &operator*() {
      if (!obj_) {
        throw std::logic_error("object is null");
      }
      return *obj_;
    }

    T *get() const { return obj_; }

    T *operator->() { return get(); }

    operator bool() const { return get(); }

    template <typename U = T> Unique(Unique<U> &&rhs) : obj_(rhs.obj_) {
      rhs.obj_ = nullptr;
    }

    template <typename U = T> Unique<T> &operator=(Unique<U> &&rhs) {
      if ((void *)&rhs != (void *)this) {
        obj_ = rhs.obj_;
        rhs.obj_ = nullptr;
      }
      return *this;
    }
  };

  template <typename T, typename... A> Unique<T> makeUnique(A &&...args) {
    auto alloc = Alloc<T>(*this);
    T *ret = new (alloc.allocate(1)) T(std::forward<A>(args)...);
    auto del = [ret, alloc = std::move(alloc)]() mutable {
      alloc.deallocate(ret, 1);
    };
    return Unique<T>(ret, del);
  }

  /** Like a `std::string` or `std::wstring`, but using our allocator */
#ifdef _WIN32
  using _C = wchar_t;
#else
  using _C = char;
#endif
  class String : public std::basic_string<_C, std::char_traits<_C>, Alloc<_C>> {
    using super = std::basic_string<_C, std::char_traits<_C>, Alloc<_C>>;

  public:
    explicit String(Memory &mem) : super(Alloc<_C>(mem)) {}
    String(String const &) = delete;
    String &operator=(String const &) = delete;
  };

  /** Like a `std::vector`, but using our allocator. */
  template <typename T> class Vec : public std::vector<T, Alloc<T>> {
    using super = std::vector<T, Alloc<T>>;

  public:
    explicit Vec(Memory &mem) : super(Alloc<T>(mem)) {}
    Vec(Vec const &) = delete;
    Vec &operator=(Vec const &) = delete;
  };

  /** Like a `std::list`, but using our allocator; prefer Vec unless the
   * mapped_type needs to be at a stable address. */
  template <typename T> class List : public std::list<T, Alloc<T>> {
    using super = std::list<T, Alloc<T>>;

  public:
    explicit List(Memory &mem) : super(Alloc<T>(mem)) {}
    List(List const &) = delete;
    List &operator=(List const &) = delete;
  };

  /** Like a `std::map`, but using our allocator.  Comparison is always
   * `std::less`. */
  template <typename K, typename T>
  class Map
      : public std::map<K, T, std::less<K>, Alloc<std::pair<K const, T>>> {
    using ValueT = std::pair<K const, T>;
    using super = std::map<K, T, std::less<K>, Alloc<ValueT>>;

  public:
    explicit Map(Memory &mem) : super(std::less<K>(), Alloc<ValueT>(mem)) {}
    Map(Map const &) = delete;
    Map &operator=(Map const &) = delete;
  };

  template <typename T> List<T> makeList() { return List<T>(*this); }

  template <typename K, typename T> Map<K, T> makeMap() {
    return Map<K, T>(*this);
  }
};

} // namespace llvm::objreader
