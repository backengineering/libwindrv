#include <libwindrv/libwindrv.h>

EXTERN_C
VOID
LibWinDrvDriverUnLoad(__in DRIVER_OBJECT *DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
}

EXTERN_C
NTSTATUS
LibWinDrvDriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    printf("hello world\n");
    return -1;
}
