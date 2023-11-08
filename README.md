# ``libwindrv``
libwindrv is a windows driver library for testing the llvm-msvc compiler when turning on Kernel CET


## Requirements

- Windows build 21389 or later
- Visual Studio 2022
- CMake 3.15+
- [WDK 11](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [ARM64 Kits](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-arm64-drivers)
- [llvm-msvc](https://github.com/backengineering/llvm-msvc/releases)

## How to enable Kernel CET
- Run [enable_cet.bat](https://github.com/backengineering/libwindrv/blob/dev/tools/enable_cet.bat) in administrator mode
- Reboot 2 times


## Building win-x64 with llvm-msvc

```
cmake -Bbuild_x64 -Ax64 -TLLVM-MSVC_v143
cmake --build build_x64 --config Release
```

## Building win-arm64 with llvm-msvc

```
cmake -Bbuild_arm64 -AARM64 -TLLVM-MSVC_v143
cmake --build build_arm64 --config Release
```

## Building win-x64 with llvm-msvc in test mode

```
cmake -Bbuild_x64 -Ax64 -TLLVM-MSVC_v143 -DENABLE_TEST=1
cmake --build build_x64 --config Release
```

## Building win-arm64 with llvm-msvc in test mode

```
cmake -Bbuild_arm64 -AARM64 -TLLVM-MSVC_v143 -DENABLE_TEST=1
cmake --build build_arm64 --config Release
```

