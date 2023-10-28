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
    return -1;
}
