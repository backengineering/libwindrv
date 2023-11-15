#include <libwindrv/libwindrv.h>

#include <intrin.h>
#include <stdint.h>
#include <ntimage.h>

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

uintptr_t
search_pattern(void *module_handle, const char *signature_value)
{
    static auto in_range = [](auto x, auto a, auto b) { return (x >= a && x <= b); };
    static auto get_bits = [](auto x) {
        return (
            in_range((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xa) : (in_range(x, '0', '9') ? x - '0' : 0));
    };
    static auto get_byte = [](auto x) { return (get_bits(x[0]) << 4 | get_bits(x[1])); };

    const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
    const auto nt_headers =
        reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(module_handle) + dos_headers->e_lfanew);

    const auto range_start = reinterpret_cast<uintptr_t>(module_handle);
    const auto range_end = range_start + nt_headers->OptionalHeader.SizeOfImage;

    auto first_match = 0ui64;
    auto pat = signature_value;

    for (uintptr_t cur = range_start; cur < range_end; cur++)
    {
        if (*pat == '\0')
        {
            return first_match;
        }
        if (*(uint8_t *)pat == '\?' || *reinterpret_cast<uint8_t *>(cur) == get_byte(pat))
        {
            if (!first_match)
                first_match = cur;

            if (!pat[2])
                return first_match;

            if (*(uint16_t *)pat == 16191 || *(uint8_t *)pat != '\?')
            {
                pat += 3;
            }
            else
            {
                pat += 2;
            }
        }
        else
        {
            pat = signature_value;
            first_match = 0;
        }
    }
    return 0u;
}

uintptr_t
get_kernel_base()
{
#ifndef _ARM64_
    const auto idtbase = *reinterpret_cast<uint64_t *>(__readgsqword(0x18) + 0x38);
    const auto descriptor_0 = *reinterpret_cast<uint64_t *>(idtbase);
    const auto descriptor_1 = *reinterpret_cast<uint64_t *>(idtbase + 8);
    const auto isr_base = ((descriptor_0 >> 32) & 0xFFFF0000) + (descriptor_0 & 0xFFFF) + (descriptor_1 << 32);
    auto align_base = isr_base & 0xFFFFFFFFFFFFF000;

    for (;; align_base -= 0x1000)
    {
        for (auto *search_base = reinterpret_cast<uint8_t *>(align_base);
             search_base < reinterpret_cast<uint8_t *>(align_base) + 0xFF9;
             search_base++)
        {
            if (search_base[0] == 0x48 && search_base[1] == 0x8D && search_base[2] == 0x1D && search_base[6] == 0xFF)
            {
                const auto relative_offset = *reinterpret_cast<int *>(&search_base[3]);
                const auto address = reinterpret_cast<uint64_t>(search_base + relative_offset + 7);
                if ((address & 0xFFF) == 0)
                {
                    if (*reinterpret_cast<uint16_t *>(address) == 0x5A4D)
                    {
                        return address;
                    }
                }
            }
        }
    }
#endif
    return 0;
}

uintptr_t
search_pattern(void *module_handle, const char *section, const char *signature_value)
{
    static auto in_range = [](auto x, auto a, auto b) { return (x >= a && x <= b); };
    static auto get_bits = [](auto x) {
        return (
            in_range((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xa) : (in_range(x, '0', '9') ? x - '0' : 0));
    };
    static auto get_byte = [](auto x) { return (get_bits(x[0]) << 4 | get_bits(x[1])); };

    const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
    const auto nt_headers =
        reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(module_handle) + dos_headers->e_lfanew);
    const auto section_headers = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt_headers + 1);

    auto range_start = 0ui64;
    auto range_end = 0ui64;
    for (auto cur_section = section_headers; cur_section < section_headers + nt_headers->FileHeader.NumberOfSections;
         cur_section++)
    {
        if (strcmp(reinterpret_cast<const char *>(cur_section->Name), section) == 0)
        {
            range_start = reinterpret_cast<uintptr_t>(module_handle) + cur_section->VirtualAddress;
            range_end = range_start + cur_section->Misc.VirtualSize;
        }
    }

    if (range_start == 0)
        return 0u;

    auto first_match = 0ui64;
    auto pat = signature_value;
    for (uintptr_t cur = range_start; cur < range_end; cur++)
    {
        if (*pat == '\0')
        {
            return first_match;
        }
        if (*(uint8_t *)pat == '\?' || *reinterpret_cast<uint8_t *>(cur) == get_byte(pat))
        {
            if (!first_match)
                first_match = cur;

            if (!pat[2])
                return first_match;

            if (*(uint16_t *)pat == 16191 || *(uint8_t *)pat != '\?')
            {
                pat += 3;
            }
            else
            {
                pat += 2;
            }
        }
        else
        {
            pat = signature_value;
            first_match = 0;
        }
    }
    return 0u;
}

static uint64_t mm_pfn_database = 0;

uintptr_t
init_mmpfn_database()
{
    auto search = search_pattern(
                      reinterpret_cast<void *>(get_kernel_base()), ".text", "B9 ? ? ? ? 48 8B 05 ? ? ? ? 48 89 43 18") +
                  5;
    auto resolved_base = search + *reinterpret_cast<int32_t *>(search + 3) + 7;
    mm_pfn_database = *reinterpret_cast<uintptr_t *>(resolved_base);
    printf("mm_pfn_database=%p\n", mm_pfn_database);
    return mm_pfn_database;
}

#pragma warning(push)
#pragma warning(disable : 4201)
struct _MMPFN
{
    uintptr_t flags;
    uintptr_t pte_address;
    uintptr_t Unused_1;
    uintptr_t Unused_2;
    uintptr_t Unused_3;
    uintptr_t Unused_4;
};
static_assert(sizeof(_MMPFN) == 0x30);

EXTERN_C
NTSYSAPI CHAR *
PsGetProcessImageFileName(__in uintptr_t Process);

typedef struct _PAGE_ENTRY
{
    union
    {
        UINT64 Flags;

        struct
        {
            UINT64 Present : 1;
            UINT64 Write : 1;
            UINT64 Supervisor : 1;
            UINT64 PageLevelWriteThrough : 1;
            UINT64 PageLevelCacheDisable : 1;
            UINT64 Accessed : 1;
            UINT64 Dirty : 1;
            UINT64 LargePage : 1;
            UINT64 Global : 1;
            UINT64 Ignored1 : 3;
            UINT64 PageFrameNumber : 36;
            UINT64 Reserved1 : 4;
            UINT64 Ignored2 : 7;
            UINT64 ProtectionKey : 4;
            UINT64 ExecuteDisable : 1;
        };
    };
} PAGE_ENTRY, *PPAGE_ENTRY;

DECLSPEC_NOINLINE
EXTERN_C
void
InitPageTable()
{
#ifndef _ARM64_

    // Copy from https://github.com/Rythorndoran/enum_real_dirbase/blob/master/enum_real_dirbase/main.cpp
    init_mmpfn_database();

    PHYSICAL_ADDRESS PA = {0};
    ULONG64 CurCr3 = (ULONG64)PAGE_ALIGN(__readcr3());
    PA.QuadPart = CurCr3;
    printf("CurCr3 pa=%p\n", PA.QuadPart);
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

                auto mem_range = MmGetPhysicalMemoryRanges();
                auto mem_range_count = 0;
                PPAGE_ENTRY cr3_ptebase = (PPAGE_ENTRY)(SelfMapIndex * 8 + PxeBase);
                printf("cr3_ptebase=%p\n", cr3_ptebase);

                auto CR3PF = cr3_ptebase->PageFrameNumber << 12;
                printf("CR3PF=%p\n", CR3PF);

                for (mem_range_count = 0; mem_range_count < 200; mem_range_count++)
                {
                    if (mem_range[mem_range_count].BaseAddress.QuadPart == 0 &&
                        mem_range[mem_range_count].NumberOfBytes.QuadPart == 0)
                        break;

                    auto start_pfn = mem_range[mem_range_count].BaseAddress.QuadPart >> 12;
                    auto end_pfn = start_pfn + (mem_range[mem_range_count].NumberOfBytes.QuadPart >> 12);

                    for (auto i = start_pfn; i < end_pfn; i++)
                    {
                        auto cur_mmpfn = reinterpret_cast<_MMPFN *>(mm_pfn_database + 0x30 * i);
                        if (cur_mmpfn->flags)
                        {
                            if (cur_mmpfn->flags == 1)
                                continue;
                            if (cur_mmpfn->pte_address != (ULONG64)cr3_ptebase)
                                continue;
                            auto decrypted_eprocess =
                                ((cur_mmpfn->flags | 0xF000000000000000) >> 0xd) | 0xFFFF000000000000;
                            auto dirbase = i << 12;
                            if (MmIsAddressValid(reinterpret_cast<void *>(decrypted_eprocess)))
                            {
                                printf(
                                    "Process -> 0x%llx\nProcessName -> %s\nDirBase -> 0x%llx\n\n",
                                    decrypted_eprocess,
                                    PsGetProcessImageFileName(decrypted_eprocess),
                                    dirbase);
                            }
                        }
                    }
                }

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
    // InitPageTable();

    return STATUS_SUCCESS;
}
