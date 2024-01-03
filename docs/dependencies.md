# Dependencies

## Dependencies

kapua has been developed to have as few external dependencies as possible. The current list is as follows:

* [boost](https://github.com/boostorg/boost)
* [openssl](https://github.com/openssl/openssl)
* [libyaml](https://github.com/yaml/libyaml)

**Build Dependencies**

* [cmake](https://github.com/Kitware/CMake)

### OpenSSL

kapua requires OpenSSL 3.2.0+, which as of Jan 2024 must be build from source:

```bash
wget https://github.com/openssl/openssl/releases/download/openssl-3.2.0/openssl-3.2.0.tar.gz
tar -xzf openssl-3.2.0.tar.gz
./config --prefix=/usr
make
make install
ldconfig
```

## Installing using a package manager

For your convenience, here are the package manager commands to install the dependencies for your platform.

**CAVEAT:** (Jan 2024) You must still download OpenSSL v3.2.0, build it from source, and install it. 

### MacOSX
```bash
brew install cmake boost yaml-cpp
```

### Alpine Linux

```bash
apk add cmake boost-dev yaml-cpp-dev
```