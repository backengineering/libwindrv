# ``libwindrv``
The libwindrv is a Windows driver library used for testing the llvm-msvc compiler when enabling Kernel CET.


## Requirements

- Windows build 21389 or later
- Visual Studio 2022
- CMake 3.15+
- [WDK 11](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [ARM64 Kits](https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/building-arm64-drivers)
- [llvm-msvc](https://github.com/backengineering/llvm-msvc/releases)

## How to enable KCET?
```
reg add HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\HypervisorEnforcedCodeIntegrity /v Enabled /t REG_DWORD /d 1 /f
reg add HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\KernelShadowStacks /v Enabled /t REG_DWORD /d 1 /f
reg add HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\KernelShadowStacks /v AuditModeEnabled /t REG_DWORD /d 1 /f
```

## How can I tell if KCET has been successfully enabled?
```C++
// If the following code causes a BSOD, it means that KCET has been successfully activated.
DECLSPEC_NOINLINE
EXTERN_C
void
KCETBSOD()
{
#ifndef _ARM64_
    __try
    {
        _asm
        {
            int 0x2D
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("except in KCETBSOD\n");
    }
#endif
}
```

## What does dump look like?
```
KERNEL_SECURITY_CHECK_FAILURE (139)
A kernel component has corrupted a critical data structure.  The corruption
could potentially allow a malicious user to gain control of this machine.
Arguments:
Arg1: 0000000000000043, A call to the secure kernel to modify shadow stack values has failed.
Arg2: 0000000000000000, Address of the trap frame for the exception that caused the BugCheck
Arg3: fffff3876cf11970, Address of the exception record for the exception that caused the BugCheck
Arg4: 0000000000000000, Reserved

fffff387`6cf118c8 fffff805`4b148b9b     : 00000000`00000139 00000000`00000043 00000000`00000000 fffff387`6cf11970 : nt!KeBugCheckEx
fffff387`6cf118d0 fffff805`4b0b886b     : 00000000`00000000 00000000`00000000 00000000`00000000 00000000`00000000 : nt!VslKernelShadowStackAssist+0xdb
fffff387`6cf11a70 fffff805`4b016b1d     : fffff387`6cf11be0 fffff387`6cf12100 00000000`00000001 00000000`00001001 : nt!KxContextToKframes+0x198dcb
fffff387`6cf11ad0 fffff805`4ae06470     : ffffffff`ffffffff fffff387`6cf12490 00000000`00000001 fffff387`6cf11be0 : nt!KeContextToKframes+0x7d
fffff387`6cf11bb0 fffff805`4b02c0fc     : 00000000`00000000 00000000`00000000 00000000`00000003 ffffffff`ffffffff : nt!KiDispatchException+0x1d0
fffff387`6cf122b0 fffff805`4b02aa05     : 00000000`00000000 fffff805`4ae74418 fffff387`6cf125c0 fffff805`4b16f127 : nt!KiExceptionDispatch+0x13c
fffff387`6cf12490 fffff805`6cdf5099     : ffffcc0e`ec7c4000 ffffcc0e`ed00957d ffffcc0e`eead9e30 00000000`497af2dc : nt!KiDebugServiceTrap+0x345
fffff387`6cf12620 ffffcc0e`ec7c4000     : ffffcc0e`ed00957d ffffcc0e`eead9e30 00000000`497af2dc fffff387`6cf10007 : testdrv1+0x5099
```


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

