#include <cassert>
#include <cstddef>
#include <functional>

namespace llvm::objreader {

template <typename T> struct Gen {
  struct Iter // models LegacyInputIterator
      : std::iterator<std::input_iterator_tag, T, std::ptrdiff_t, T *, T &> {
    Gen<T> *gen_{nullptr};
    Iter() = default;
    explicit Iter(Gen<T> *gen) : gen_(gen) {}
    T &operator*() { return gen_->val_; }
    T *operator->() { return gen_->val_; }
    bool operator==(Iter const &rhs) const {
      return (gen_ == nullptr || gen_->done_) && (!rhs.gen_);
    }
    bool operator!=(Iter const &rhs) const { return !operator==(rhs); }
    Iter operator++() { return gen_->next(); }
  };

  T val_{};
  std::function<bool(T &)> next_;
  bool done_;

  Iter next() {
    done_ = next_(val_);
    return Iter{this};
  }

  Iter begin() { return next(); }
  Iter end() { return Iter{nullptr}; }

  /** The `next` function (such as a capturing lambda) will move to the next
   * element (updating internal state and such) and assign it via the given
   * reference. It will return true if there are no more elements.
   */
  Gen(std::function<bool(T &)> next) : next_(next) {}
};

} // namespace llvm::objreader
