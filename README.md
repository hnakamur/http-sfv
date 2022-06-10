http-sfv
========

This library is an implementation for [RFC 8941: Structured Field Values for HTTP](https://www.rfc-editor.org/rfc/rfc8941).

## How to build

Install code formatters.

```
sudo apt-get update \
sudo apt-get install -y clang-format cmake-format
```

Generate Makefile.

```
mkdir build
cd build
cmake ..
```

Build and run tests

```
make && ./tests
```
