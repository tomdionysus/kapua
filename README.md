# kapua

A fully decentralised storage, compute, database and networking platform, in C++11.

## Building

### Dependencies:

* [cmake](https://github.com/Kitware/CMake)
* [curl](https://github.com/curl/curl)
* [libyaml](https://github.com/yaml/libyaml)

**Debain/Ubuntu/etc**

```sh
apt-get install cmake curl libyaml
```

**MacOS**

```sh
brew install curl yaml-cpp
```

### Build

```sh
git clone https://github.com/tomdionysus/kapua
cd kapua
mkdir build
cd build
cmake ..
make
```

## Documentation

* [Project Goals](docs/goals.md)
* [Philosophy](docs/philosophy.md)
* [Infrastructure](docs/infrastructure.md)
* [Routing](docs/routing.md)
* [Storage](docs/storage.md)

## License

Kapua is licensed under the [MIT License](https://en.wikipedia.org/wiki/MIT_License). Please see [LICENSE](LICENSE) for details.