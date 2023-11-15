#include <libwindrv/libwindrv.h>

#include <intrin.h>

DECLSPEC_NOINLINE
EXTERN_C
VOID
main2();

EXTERN_C
VOID
LibWinDrvDriverUnLoad(__in DRIVER_OBJECT *DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    printf("world hello\n");
}

// ULONG_PTR g_addr = 0;
//__declspec(naked) __declspec("volatile") void myfunc2()
//{
//     _asm {
//         mov rax, g_addr
//         add rax, 3
//         jmp rax
//     }
// }
//
//__declspec(naked) __declspec("volatile") void myfunc1(void *p)
//{
//     _asm {
//         mov rax, [rsp]
//         mov g_addr, rax
//         mov [rsp], rcx
//         ret
//     }
// }

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

DECLSPEC_NOINLINE
EXTERN_C
void
InitPageTable()
{
#ifndef _ARM64_
    PHYSICAL_ADDRESS PA = {0};
    ULONG64 CurCr3 = (ULONG64)PAGE_ALIGN(__readcr3());
    PA.QuadPart = CurCr3;
    PVOID MapAddress = MmGetVirtualForPhysical(PA);
    if (MapAddress)
    {
        for (int i = 0; i < 512; i++)
        {
            if ((*(PULONG64)((ULONG64)MapAddress + i * 8) & 0x0000fffffffff000) == (CurCr3 & 0x0000fffffffff000))
            {
                ULONG64 SelfMapIndex = i;
                ULONG64 PteBase = (ULONG64)(0xffff000000000000 + (SelfMapIndex << 39));
                ULONG64 PdeBase = PteBase + (SelfMapIndex << 30);
                ULONG64 PpeBase = PdeBase + (SelfMapIndex << 21);
                ULONG64 PxeBase = PpeBase + (SelfMapIndex << 12);
                printf("SelfMapIndex=%d\n", SelfMapIndex);
                printf("PteBase=%p\n", PteBase);
                printf("PdeBase=%p\n", PdeBase);
                printf("PpeBase=%p\n", PpeBase);
                printf("PxeBase=%p\n", PxeBase);
                break;
            }
        }
    }
#endif
}

EXTERN_C
NTSTATUS
LibWinDrvDriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    printf("hello world\n");
    /*myfunc1((void *)myfunc2);
    _asm {
        nop
        nop
        nop
    }
    printf("hello world2\n");*/
    // KCETBSOD();
    main2();
    InitPageTable();

    return STATUS_SUCCESS;
}
