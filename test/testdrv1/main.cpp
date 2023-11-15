#include <libwindrv/libwindrv.h>

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

    return STATUS_SUCCESS;
}
