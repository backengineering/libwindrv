#pragma once
#include <fltKernel.h>
#include <stdlib.h>

#include <libwindrv/crt/libwindrv.crt.h>

extern "C" PDRIVER_OBJECT LibWinDrvDriverObject;
extern "C" wchar_t LibWinDrvRegistryKey[_MAX_PATH];
extern "C" wchar_t LibWinDrvServiceKeyName[_MAX_PATH];
extern "C" PVOID LibWinDrvImageBase;
extern "C" SIZE_T LibWinDrvImageSize;
extern "C" DRIVER_INITIALIZE LibWinDrvDriverEntry;
extern "C" DRIVER_UNLOAD LibWinDrvDriverUnLoad;
