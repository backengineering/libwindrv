#include <libwindrv/libwindrv.h>

EXTERN_C
VOID
LibWinDrvDriverUnLoad(__in DRIVER_OBJECT *DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    printf("world hello\n");
}

EXTERN_C
NTSTATUS
LibWinDrvDriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    printf("hello world\n");
    return -1;
}
