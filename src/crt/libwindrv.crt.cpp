#include <libwindrv/crt/libwindrv.crt.h>
#include <libwindrv/libwindrv.h>

#include <stdarg.h>

extern "C" int __cdecl printf(const char *format, ...)
{
    NTSTATUS Status;
    va_list Args;

    va_start(Args, format);
    Status = vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, Args);
    va_end(Args);

    return NT_SUCCESS(Status);
}

extern "C" int __cdecl puts(const char *_Str)
{
    return DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, _Str);
}
