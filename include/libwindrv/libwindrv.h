#pragma once
#include <fltKernel.h>
#include <stdlib.h>

#include <libwindrv/crt/libwindrv.crt.h>

EXTERN_C
PDRIVER_OBJECT LibWinDrvDriverObject;
EXTERN_C
wchar_t LibWinDrvRegistryKey[_MAX_PATH];
EXTERN_C
wchar_t LibWinDrvServiceKeyName[_MAX_PATH];
EXTERN_C
PVOID LibWinDrvImageBase;
EXTERN_C
SIZE_T LibWinDrvImageSize;
EXTERN_C
DRIVER_INITIALIZE LibWinDrvDriverEntry;
EXTERN_C
DRIVER_UNLOAD LibWinDrvDriverUnLoad;
