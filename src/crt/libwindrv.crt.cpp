#include <libwindrv/crt/libwindrv.crt.h>
#include <libwindrv/libwindrv.h>

#include <stdarg.h>

extern "C" int __cdecl printf(const char *format, ...)
{
    NTSTATUS Status;
    va_list Args;

    va_start(Args, format);
    Status = vDbgPrintEx(DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL, format, Args);
    va_end(Args);

    return NT_SUCCESS(Status);
}

extern "C" int __cdecl dprintf(const char *format, ...)
{
    NTSTATUS Status;
    va_list Args;

    va_start(Args, format);
    Status = vDbgPrintEx(DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL, format, Args);
    va_end(Args);

    return NT_SUCCESS(Status);
}

extern "C" int __cdecl puts(const char *_Str)
{
    char Temp[] = {117, 23, 0};
    Temp[0] ^= 80;
    Temp[1] ^= 100;
    return printf(Temp, _Str);
}
