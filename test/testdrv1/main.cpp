#include <libwindrv/libwindrv.h>

EXTERN_C
VOID
LibWinDrvDriverUnLoad(__in DRIVER_OBJECT *DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    dprintf("world hello\n");
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

EXTERN_C
NTSTATUS
LibWinDrvDriverEntry(__in DRIVER_OBJECT *DriverObject, __in UNICODE_STRING *RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    dprintf("hello world\n");
    /*myfunc1((void *)myfunc2);
    _asm {
        nop
        nop
        nop
    }
    dprintf("hello world2\n");*/

    __try
    {
        _asm
        {
            int 0x2D
        }
    }
    __except (1)
    {
        dprintf("except\n");
    }

    return -1;
}
