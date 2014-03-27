# xpp - A C++11 RAII wrapper for XCB

## Synopsis

XPP is a C++11
[RAII](https://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization)
wrapper around [X protocol C-language Binding
(XCB)](http://xcb.freedesktop.org). Pointers to dynamically allocated memory,
such as events and errors are wrapped in std::shared_ptr.

Furthermore, interfaces for connection and resource types are provided to
facilitate the creation of custom classes. For convenience, a connection class
and several basic resource type classes are readily available.

XPP makes widespread use of the
[Curiously Recurring Template Pattern (CRTP)](https://en.wikibooks.org/wiki/More_C++_Idioms/Curiously_Recurring_Template_Pattern)
to avoid overhead through dynamic dispatch. Hence, most interfaces are
implicitly defined.

### Prerequisites
  * Python 2
  * GCC >= 4.8 (or Clang >= 3.3, untested)
  * libxcb

### Quick Start

```
  git clone https://github.com/jrk-/xpp
  cd xpp
  make
  make examples
  cd src/examples
  for demo in demo_*; do ./${demo}; done
```

### Documentation

The bindings can be generated by calling `make` in the top level directory. If
this fails, check the `XCBGEN` and `PROTODIR` variables in
[src/proto/Makefile](src/proto/Makefile). These need to point to the `xcbgen`
python package and the xml protocol description respectively.

Currently there is no documentation available.
Until then please refer to the source.

Recent (and working) examples can be found in [src/examples](src/examples).
To compile them, call `make examples` in the `xpp` directory or just `make` in
[src/examples](src/examples).
