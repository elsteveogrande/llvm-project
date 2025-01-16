# libobjreader

Yet another binary file and debug-information parser.
As of this writing, perhaps the third or so, in `llvm-project`.  :)

## Why?

So, why another one of these things in llvm-project?

We need to be able to decode debug information in multiple 
different platforms at runtime, for C++23 `<stacktrace>`
support.  LLDB has an excellent implementation (of course 
because it's a excellent debugger).  This project also contains 
quite a few other utilities (like `llvm-dwarfdump`, 
`obj2yaml`, ...).  Some of these make use of a "common" library 
in `include/llvm/DebugInfo`, but that has dependencies 
spidering throughout LLVM, making it hard to use in `libcxx`.  
And while `DebugInfo` and other libraries within `llvm-project` 
can easily handle the relatively simple use of `stacktrace`, we 
need a few other things apart from functionality:

(1) we need to be able to include it into `libcxx`;
(2) caller-provided allocator support (part of `stacktrace`, and so
    the debug-info lib it uses should offer that too);
(3) the library should probably be leaner (but that's not a
    show-stopper);
(4) it must have minimal dependencies, and it definitely cannot
    depend on an external project;
(5) should *probably* try to be signal-safe (allocations aside),
    since `stacktrace` could conceivably be used in situations
    where an error is being reported, and the calling code is in
    a signal handler (although C++23 doesn't mandate signal-safety).

## Goals

* Immediate goal: support for reading multiple object file types:
  this means binaries, libraries, and "companion" debug files:
  - Mach-O (Mac OSX); the executable along with its `.dSYM` directory
    alongside it, containing DWARF debugging data
  - ELF format files (Linux and others); the executable and `.dwo`
    companion file(s) (also DWARF);
  - PE and PDB formats (Windows), which unfortunately don't have overlap
    like the above two, but is more or less similar in principle.
* Eventually: make this available throughout `llvm-project`:
  - This should go into one commonly-accessible location.  (And 
  by the way not its current location, buried within the stacktrace
  implementation.)
  - Hopefully, this will be flexible enough so that we could consolidate 
  the multiple places where e.g. Mach-O and DWARF formats are decoded!

## Non-goals

* Writing object files is not a goal (we only need read support).
