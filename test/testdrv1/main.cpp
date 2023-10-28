#include <libwindrv/libwindrv.h>

EXTERN_C
void
LibWinDrvDriverUnLoad(__in DRIVER_OBJECT *driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
}

EXTERN_C
NTSTATUS
LibWinDrvDriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    printf("hello world\n");
    return -1;
}
