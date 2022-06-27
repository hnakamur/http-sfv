http-sfv
========

This library is an implementation for [RFC 8941: Structured Field Values for HTTP](https://www.rfc-editor.org/rfc/rfc8941) written in C.
The API is not frozen yet and may change in the future.

## How to build

Install code formatters.

```
sudo apt-get update \
sudo apt-get install -y clang-14 clang-format cmake-format
```

Generate Makefile.

```
. env
mkdir build
cd build
cmake ..
```

Build and run tests

```
make -j check
```

It also runs test cases defined in [httpwg/structured-field-tests: Tests for HTTP Structured Field Values](https://github.com/httpwg/structured-field-tests).
