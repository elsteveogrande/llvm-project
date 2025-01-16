//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

#include <experimental/stacktrace>

#include <cassert>

/*
  (19.6.4.2)

  // [stacktrace.basic.cons], creation and assignment
  static basic_stacktrace current(const allocator_type& alloc = allocator_type()) noexcept;   [1]
  static basic_stacktrace current(size_type skip,
                                  const allocator_type& alloc = allocator_type()) noexcept;   [2]
  static basic_stacktrace current(size_type skip, size_type max_depth,
                                  const allocator_type& alloc = allocator_type()) noexcept;   [3]

  basic_stacktrace() noexcept(is_nothrow_default_constructible_v<allocator_type>);            [4]
  explicit basic_stacktrace(const allocator_type& alloc) noexcept;                            [5]

  basic_stacktrace(const basic_stacktrace& other);                                            [6]
  basic_stacktrace(basic_stacktrace&& other) noexcept;                                        [7]
  basic_stacktrace(const basic_stacktrace& other, const allocator_type& alloc);               [8]
  basic_stacktrace(basic_stacktrace&& other, const allocator_type& alloc);                    [9]
  basic_stacktrace& operator=(const basic_stacktrace& other);                                 [10]
  basic_stacktrace& operator=(basic_stacktrace&& other)
    noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
      allocator_traits<Allocator>::is_always_equal::value);                                   [11]

  ~basic_stacktrace();                                                                        [12]
*/

template <typename T>
struct test_alloc {
  using size_type     = size_t;
  using value_type    = T;
  using pointer       = T*;
  using const_pointer = T const*;

  template <typename U>
  struct rebind {
    using other = test_alloc<U>;
  };

  size_t* alloc_counter_;
  size_t* dealloc_counter_;
  std::allocator<T> wrapped_{};

  explicit test_alloc(size_t* alloc_counter, size_t* dealloc_counter)
      : alloc_counter_(alloc_counter), dealloc_counter_(dealloc_counter) {}

  T* allocate(size_t n) {
    *alloc_counter_ += n;
    return wrapped_.allocate(n);
  }

  void deallocate(T* ptr, size_t n) {
    *dealloc_counter_ += n;
    return wrapped_.deallocate(ptr, n);
  }

  auto allocate_at_least(size_t n) {
    auto ret = wrapped_.allocate_at_least(n);
    *alloc_counter_ += ret.count;
    return ret;
  }
};

// clang-format off
uint32_t test1_line;
uint32_t test2_line;

template <class A>
[[noinline]] std::basic_stacktrace<A> test1(A& alloc) {
  test1_line = __LINE__ + 1; // add 1 to get the next line (where the call to `current` occurs)
  auto ret = std::basic_stacktrace<A>::current(alloc);
  return ret;
}

template <class A>
[[noinline]] std::basic_stacktrace<A> test2(A& alloc) {
  test2_line = __LINE__ + 1; // add 1 to get the next line (where the call to `current` occurs)
  auto ret = test1(alloc);
  return ret;
}
// clang-format on

/*
    [1]
    static basic_stacktrace current(const allocator_type& alloc = allocator_type()) noexcept;

    Returns: A basic_stacktrace object with frames_ storing the stacktrace of the current evaluation
    in the current thread of execution, or an empty basic_stacktrace object if the initialization of
    frames_ failed. alloc is passed to the constructor of the frames_ object.
    [Note 1: If the stacktrace was successfully obtained, then frames_.front() is the stacktrace_entry
    representing approximately the current evaluation, and frames_.back() is the stacktrace_entry
    representing approximately the initial function of the current thread of execution. — end note]
  */
void test_current_with_alloc() {
  size_t allocs   = 0;
  size_t deallocs = 0;
  {
    test_alloc<std::stacktrace_entry> alloc{&allocs, &deallocs};
    // uint32_t main_line = __LINE__ + 1;
    auto st = test2(alloc);

    assert(st.size() >= 3);
    assert(st[0]);
    assert(st[0].native_handle());
    // assert(st[0].description);
    // assert(st[0].source_file.ends_with("basic.cons.pass.cpp"));
    // assert(st[0].source_line == test1_line);
    assert(st[1]);
    assert(st[1].native_handle());
    // assert(st[1].description);
    // assert(st[1].source_file.ends_with("basic.cons.pass.cpp"));
    // assert(st[1].source_line == test2_line);
    assert(st[2]);
    assert(st[2].native_handle());
    // assert(st[2].description);
    // assert(st[2].source_file.ends_with("basic.cons.pass.cpp"));
    // assert(st[2].source_line == main_line);

    // Verify it used the allocator we provided
    printf("@@@ + %lu, - %lu\n", allocs, deallocs);
    assert(allocs);
    assert(!deallocs);

    // Note: also testing destructor [12].
    // st going out of scope here.
  }
  assert(deallocs == allocs);
}

/*
  [2]
  static basic_stacktrace current(size_type skip,
                              const allocator_type& alloc = allocator_type()) noexcept;
  Let t be a stacktrace as-if obtained via basic_stacktrace​::​current(alloc). Let n be t.size().
  Returns: A basic_stacktrace object where frames_ is direct-non-list-initialized from arguments
  t.begin() + min(n, skip), t.end(), and alloc, or an empty basic_stacktrace object if the
  initialization of frames_ failed.
*/
void test_current_with_skip() {
  // Use default allocator for simplicity; alloc is covered above
  auto st_skip0 = std::stacktrace::current();
  assert(st_skip0.size() >= 2);
  auto st_skip1 = std::stacktrace::current(1);
  assert(st_skip1.size() >= 1);
  assert(st_skip0.size() == st_skip1.size() + 1);
  assert(st_skip0[1] == st_skip1[0]);
  auto st_skip_many = std::stacktrace::current(1 << 20);
  assert(st_skip_many.empty());
}

/*
  [3]
  static basic_stacktrace current(size_type skip, size_type max_depth,
                              const allocator_type& alloc = allocator_type()) noexcept;
  Let t be a stacktrace as-if obtained via basic_stacktrace​::​current(alloc). Let n be t.size().
  Preconditions: skip <= skip + max_depth is true.
  Returns: A basic_stacktrace object where frames_ is direct-non-list-initialized from arguments
  t.begin() + min(n, skip), t.begin() + min(n, skip + max_depth), and alloc, or an empty
  basic_stacktrace object if the initialization of frames_ failed.
*/
void test_current_with_skip_depth() {
  auto st  = std::stacktrace::current();
  auto top = *st.begin();
  assert(st.size() >= 2);
  st = std::stacktrace::current(0, 2);
  assert(st.size() == 2);
  assert(*st.begin() == top);
  st = std::stacktrace::current(0, 1);
  assert(st.size() == 1);
  assert(*st.begin() == top);
}

/*
  [4]
  basic_stacktrace() noexcept(is_nothrow_default_constructible_v<allocator_type>);
  Postconditions: empty() is true.
*/
void test_default_construct() {
  std::stacktrace st;
  assert(st.empty());
}

/*
  [5]
  explicit basic_stacktrace(const allocator_type& alloc) noexcept;
  Effects: alloc is passed to the frames_ constructor.
  Postconditions: empty() is true.
*/
void test_construct_with_allocator() {
  size_t allocs   = 0;
  size_t deallocs = 0;
  test_alloc<std::stacktrace_entry> alloc(&allocs, &deallocs);
  std::basic_stacktrace<decltype(alloc)> st(alloc);
  assert(st.empty());
  assert(allocs == 0);
  std::__stacktrace_helper::__append(st, {});
  assert(!st.empty());
  assert(allocs > 0);
}

/*
  [6] basic_stacktrace(const basic_stacktrace& other);
  [7] basic_stacktrace(basic_stacktrace&& other) noexcept;
  [8] basic_stacktrace(const basic_stacktrace& other, const allocator_type& alloc);
  [9] basic_stacktrace(basic_stacktrace&& other, const allocator_type& alloc);
  [10] basic_stacktrace& operator=(const basic_stacktrace& other);
  [11] basic_stacktrace& operator=(basic_stacktrace&& other)
    noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
    allocator_traits<Allocator>::is_always_equal::value);
  
  Remarks: Implementations may strengthen the exception specification for these functions
  ([res.on.exception.handling]) by ensuring that empty() is true on failed allocation.
*/
void test_copy_move_ctors() {
  using A = std::allocator<std::stacktrace_entry>;
  A alloc;
  auto st = std::basic_stacktrace<A>::current(alloc);

  auto copy_constr = std::basic_stacktrace<A>(st);
  assert(st == copy_constr);

  std::basic_stacktrace<A> copy_assign;
  copy_assign = std::basic_stacktrace<A>(st);
  assert(st == copy_assign);

  auto st2 = test2(alloc);
  assert(st2.size());
  std::basic_stacktrace<A> move_constr(std::move(st2));
  assert(move_constr.size());
  assert(!st2.size());

  auto st3 = test2(alloc);
  assert(st3.size());
  std::basic_stacktrace<A> move_assign;
  move_assign = std::move(st3);
  assert(move_assign.size());
  assert(!st3.size());

  // TODO: test cases with `select_on_container_copy_construction`
}

int main(int, char**) {
  test_current_with_alloc();
  test_current_with_skip();
  test_current_with_skip_depth();

  return 0;
}
