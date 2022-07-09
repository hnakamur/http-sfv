http-sfv
========

This library is an implementation for [RFC 8941: Structured Field Values for HTTP](https://www.rfc-editor.org/rfc/rfc8941) written in C.
The API is not frozen yet and may change in the future.

A function for parsing values of [RFC 9213: Targeted HTTP Cache Control](https://www.rfc-editor.org/rfc/rfc9213.html) is included.
This function requires no memory allocation.

I wrote a blog entry in Japanese at [Targeted Cache Control のライブラリをC言語で書いた · hnakamur's blog](https://hnakamur.github.io/blog/2022/07/09/targeted-cache-control-impl/).

## How to build

Install clang and code formatters. Run the following commands on a Ubuntu 22.04 LTS machine:

```
sudo apt-get update \
sudo apt-get install -y clang-14 clang-format-14 cmake-format
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
