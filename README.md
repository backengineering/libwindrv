# ``libwindrv``
libwindrv is a windows driver library for testing the llvm-msvc compiler when turning on CET


## Requirements

- Visual Studio 2022
- CMake 3.15+
- [WDK 11](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [ARM64 Kit](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-arm64-drivers)
- [llvm-msvc](https://github.com/backengineering/llvm-msvc/releases)

## Building win-x64 with llvm-msvc

```
cmake -Bbuild -TLLVM-MSVC_v143 -DCMAKE_WIN64_DRIVER=1
cmake --build build --config Release
```

## Building win-arm64 with llvm-msvc

```
cmake -Bbuild_arm64 -TLLVM-MSVC_v143 -DCMAKE_ARM64_DRIVER=1
cmake --build build_arm64 --config Release
```

## Building win-x64 with llvm-msvc in test mode

```
cmake -Bbuild -TLLVM-MSVC_v143 -DCMAKE_WIN64_DRIVER=1 -DENABLE_TEST=1
cmake --build build --config Release
```

## Building win-arm64 with llvm-msvc in test mode

```
cmake -Bbuild_arm64 -TLLVM-MSVC_v143 -DCMAKE_ARM64_DRIVER=1 -DENABLE_TEST=1
cmake --build build_arm64 --config Release
```

