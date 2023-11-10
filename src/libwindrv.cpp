#include <libwindrv/libwindrv.h>

#include <ntimage.h>
#define NTSTRSAFE_LIB
#define NTSTRSAFE_NO_CB_FUNCTIONS
#include <ntstrsafe.h>

typedef struct _KLDR_DATA_TABLE_ENTRY_COMMON
{
    LIST_ENTRY InLoadOrderLinks;
    PVOID ExceptionTable;
    ULONG ExceptionTableSize;
    // ULONG padding on IA64
    PVOID GpValue;
    PNON_PAGED_DEBUG_INFO NonPagedDebugInfo;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT __Unused5;
    PVOID SectionPointer;
    ULONG CheckSum;
    // ULONG padding on IA64
    PVOID LoadedImports;
    PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY_COMMON, *PKLDR_DATA_TABLE_ENTRY_COMMON;

extern "C" PDRIVER_OBJECT LibWinDrvDriverObject = nullptr;
extern "C" wchar_t LibWinDrvRegistryKey[_MAX_PATH];
extern "C" wchar_t LibWinDrvServiceKeyName[_MAX_PATH];
extern "C" PVOID LibWinDrvImageBase = nullptr;
extern "C" SIZE_T LibWinDrvImageSize = 0;

VOID
DriverUnload(__in DRIVER_OBJECT *DriverObject)
{
    LibWinDrvDriverUnLoad(DriverObject);

    // TODO
}

EXTERN_C
NTSTATUS
DriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    // TODO

    DriverObject->DriverUnload = (DRIVER_UNLOAD *)(DriverUnload);
    LibWinDrvDriverObject = DriverObject;

    do
    {
        KLDR_DATA_TABLE_ENTRY_COMMON *pEntry = (KLDR_DATA_TABLE_ENTRY_COMMON *)(LibWinDrvDriverObject->DriverSection);
        if (pEntry)
        {
            LibWinDrvImageBase = pEntry->DllBase;
            LibWinDrvImageSize = (SIZE_T)pEntry->SizeOfImage;

            // TODO

            DriverObject->Flags |= 0x20;
        }
        else
        {
            Status = STATUS_VIRUS_DELETED;
            break;
        }

        if (RegistryPath)
        {
            // TODO
        }
        else
        {
            Status = STATUS_VIRUS_INFECTED;
            break;
        }

        Status = LibWinDrvDriverEntry(DriverObject, RegistryPath);

    } while (0);

    return Status;
}
