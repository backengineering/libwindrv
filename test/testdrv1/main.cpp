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
void __writecr2(ULONG_PTR);

EXTERN_C
void
_sgdt(void *Destination);

EXTERN_C
unsigned char
_xtest(void);

EXTERN_C
void
Testintrin()
{
#ifndef _ARM64_
    // test cr0
    {
        auto cr0 = __readcr0();
        printf("readcr0 cr0=%p\n", cr0);
        __writecr0(cr0);
        cr0 = __readcr0();
        printf("writecr0 cr0=%p\n", cr0);
    }

    // test cr2
    {
        auto cr2 = __readcr2();
        printf("readcr2 cr2=%p\n", cr2);
        __writecr2(cr2);
        cr2 = __readcr2();
        printf("writecr2 cr2=%p\n", cr2);
    }

    // test cr3
    {
        auto cr3 = __readcr3();
        printf("readcr3 cr3=%p\n", cr3);
        __writecr3(cr3);
        cr3 = __readcr3();
        printf("writecr3 cr3=%p\n", cr3);
    }

    // test cr4
    {
        auto cr4 = __readcr4();
        printf("readcr4 cr4=%p\n", cr4);
        __writecr4(cr4);
        cr4 = __readcr4();
        printf("writecr4 cr4=%p\n", cr4);
    }

    // test cr8
#    ifdef _WIN64
    {
        auto cr8 = __readcr8();
        printf("readcr8 cr8=%p\n", cr8);
        __writecr8(cr8);
        cr8 = __readcr8();
        printf("writecr8 cr8=%p\n", cr8);
    }
#    endif

    // test dr
    {
        auto dr0 = __readdr(0);
        printf("read dr0=%p\n", dr0);
        __writedr(0, dr0);
        dr0 = __readdr(0);
        printf("write dr0=%p\n", dr0);

        auto dr7 = __readdr(7);
        printf("read dr7=%p\n", dr7);
        __writedr(0, dr7);
        dr7 = __readdr(7);
        printf("write dr7=%p\n", dr7);
    }

    // test msr
    {
        auto msr_C0000082 = __readmsr(0xC0000082);
        printf("read msr 0xC0000082 =%p\n", msr_C0000082);

        __writemsr(0xC0000082, 0);
        auto msr_C0000082_temp = __readmsr(0xC0000082);
        printf("write msr 0xC0000082 =%p\n", msr_C0000082_temp);

        __writemsr(0xC0000082, msr_C0000082);
        msr_C0000082 = __readmsr(0xC0000082);
        printf("write msr 0xC0000082 =%p\n", msr_C0000082);
    }

    // test cpuid
    {
        int a[4] = {0};
        __cpuid(a, 1);
        auto ecx = a[2];
        if (ecx & 0x80000000)
        {
            printf("cpuid: You are in virtual machine!\n");
        }

        memset(a, 0, sizeof(a));
        // CPUID_ADDR_WIDTH 0x80000008
        __cpuid(a, 0x80000008);
        unsigned int AddrWidth = ((a[0] >> 8) & 0x0ff);
        printf("cpuid: AddrWidth=%d\n", AddrWidth);
    }

    // test cpuidex
    {
        int a[4] = {0};
        __cpuidex(a, 1, 0);
        auto ecx = a[2];
        if (ecx & 0x80000000)
        {
            printf("cpuidex: You are in virtual machine!\n");
        }

        memset(a, 0, sizeof(a));
        // CPUID_ADDR_WIDTH 0x80000008
        __cpuidex(a, 0x80000008, 0);
        unsigned int AddrWidth = ((a[0] >> 8) & 0x0ff);
        printf("cpuidex: AddrWidth=%d\n", AddrWidth);
    }

    // test xbeign/xend
    {
        printf("test xbeign/xend\n");
        __try
        {
            _xbegin();
            _xend();
        }
        __except (1)
        {
            printf("test xbeign/xend goto except handler\n");
        }
    }

    // test stosb
    {
        unsigned char c = 0x40; /* '@' character */
        unsigned char s[] = "*********************************";

        printf("%s\n", s);
        __stosb((unsigned char *)s + 1, c, 6);
        printf("%s\n", s);
        //*********************************
        //*@@@@@@**************************
    }

    // test stosw
    {
        unsigned short val = 128;
        unsigned short a[100];
        memset(a, 0, sizeof(a));
        __stosw(a + 10, val, 2);
        printf("%u %u %u %u\n", a[9], a[10], a[11], a[12]);
        // 0 128 128 0
    }

    // test stosd
    {
        unsigned long val = 99999;
        unsigned long a[10];

        memset(a, 0, sizeof(a));
        __stosd(a + 1, val, 2);

        printf("%u %u %u %u\n", a[0], a[1], a[2], a[3]);
        // 0 99999 99999 0
    }

#    ifdef _WIN64
    // test stosq
    {
        unsigned __int64 val = 0xFFFFFFFFFFFFI64;
        unsigned __int64 a[10];
        memset(a, 0, sizeof(a));
        __stosq(a + 1, val, 2);
        printf("%I64x %I64x %I64x %I64x\n", a[0], a[1], a[2], a[3]);
        // 0 ffffffffffff ffffffffffff 0
    }
#    endif

    // test readpmc
    {
        auto pmc0 = __readpmc(0);
        printf("pmc0:%I64x\n", pmc0);
    }

    // test readtscp
    {
        unsigned int aux = 1;
        auto tickcount = __rdtscp(&aux);
        printf("aux:0x%x\n", aux);
        printf("tickcount:%I64x\n", tickcount);
    }

    // test inbyte
    {
        ///* VMware I/O Port  */
        __try
        {
            auto PortRead = __inbyte(5658);
            __outbyte(5658, PortRead);
            printf("inbyte:%x\n", PortRead);
        }
        __except (1)
        {
            printf("inbyte in except hanlder\n");
        }
    }

    // test inword
    {
        ///* VMware I/O Port  */
        __try
        {
            auto PortRead = __inword(5658);
            __outword(5658, PortRead);
            printf("inword:%x\n", PortRead);
        }
        __except (1)
        {
            printf("inword in except hanlder\n");
        }
    }

    // test indword
    {
        ///* VMware I/O Port  */
        __try
        {
            auto PortRead = __indword(5658);
            __outdword(5658, PortRead);
            printf("indword:%x\n", PortRead);
        }
        __except (1)
        {
            printf("indword in except hanlder\n");
        }
    }

    // test invlpg
    {
        auto mem = ExAllocatePool(NonPagedPool, 0x1000);
        if (mem)
        {
            __invlpg(mem);
            printf("__invlpg addr=%p\n", mem);
            ExFreePool(mem);
        }
    }

    // test invpcid
    {
        ULONG_PTR pcid = 11;
        _invpcid(2, &pcid);
        printf("_invpcid pcid=%p\n", pcid);
    }

    // test inbytestring
    {
        ///* VMware I/O Port  */
        char buf[100] = {0};
        __inbytestring(5658, (PUCHAR)buf, sizeof(buf));
        __outbytestring(5658, (PUCHAR)buf, sizeof(buf));
        printf("buf1=%s\n", buf);
    }

    // test __inwordstring
    {
        ///* VMware I/O Port  */
        unsigned short buf = 2;
        __inwordstring(5658, &buf, sizeof(buf));
        __outwordstring(5658, &buf, sizeof(buf));
        printf("buf2=%d\n", buf);
    }

    // test __indwordstring
    {
        ///* VMware I/O Port  */
        unsigned long buf = 2;
        __indwordstring(5658, &buf, sizeof(buf));
        __outdwordstring(5658, &buf, sizeof(buf));
        printf("buf3=%d\n", buf);
    }

    // test cti/sti
    {
        printf("test cti/sti begin\n");
        _disable();
        _enable();
        printf("test cti/sti end\n");
    }

    // test lsl
    {
        printf("test lsl begin\n");

#    define EFLAGS_ZF 0x00000040
#    define KGDT_R3_DATA 0x0020
#    define RPL_MASK 0x3

#    ifdef _M_IX86
        typedef unsigned int READETYPE;
#    else
        typedef unsigned __int64 READETYPE;
#    endif

        const unsigned long initsl = 0xbaadbabe;
        READETYPE eflags = 0;
        unsigned long sl = initsl;

        printf("Before: segment limit =0x%x eflags =0x%x\n", sl, eflags);
        sl = __segmentlimit(KGDT_R3_DATA + RPL_MASK);

        eflags = __readeflags();
        printf("eflags=%p\n", eflags);
        __writeeflags(eflags & ~EFLAGS_ZF);
        auto eflags2 = __readeflags();
        __writeeflags(eflags);
        printf("eflags2=%p\n", eflags2);

        printf(
            "After: segment limit =0x%x eflags =0x%x eflags.zf = %s\n",
            sl,
            eflags,
            (eflags & EFLAGS_ZF) ? "set" : "clear");

        // If ZF is set, the call to lsl succeeded; if ZF is clear, the call failed.
        printf("%s\n", eflags & EFLAGS_ZF ? "Success!" : "Fail!");

        // You can verify the value of sl to make sure that the instruction wrote to it
        printf("sl was %s\n", (sl == initsl) ? "unchanged" : "changed");

        printf("test lsl end\n");

        /*
        Before: segment limit =0xbaadbabe eflags =0x0
        After: segment limit =0xffffffff eflags =0x256 eflags.zf = set
        Success!
        sl was changed
        */
    }

    // test wbinvd
    {
        printf("test wbinvd begin\n");
        __wbinvd();
        printf("test wbinvd end\n");
    }

    {
        auto eflags = __readeflags();
        printf("eflags 3389 =%p\n", eflags);
    }

    struct Idtr
    {
        unsigned short limit;
        ULONG_PTR base;
    };
    // test sidt/lidt
    {
        Idtr IDT{};
        __sidt(&IDT);
        __lidt(&IDT);
        printf("test sidt/lidt end\n");
    }

    {
        auto eflags = __readeflags();
        printf("eflags 007 =%p\n", eflags);
    }

    // test gdtr
    {
        using Gdtr = Idtr;
        Gdtr gdtr = {};
        _sgdt(&gdtr);
        printf("test _sgdt end\n");
    }

    {
        auto eflags = __readeflags();
        printf("eflags 996 =%p\n", eflags);
    }

    // test __readgsbyte
    {
        auto old = __readgsbyte(0x10);
        printf("test gs  __readgsbyte(0x10)=0x%x\n", old);
        __writegsbyte(0x10, 0xcc);
        __incgsbyte(0x10);
        __addgsbyte(0x10, 1);
        printf("test gs  __readgsbyte(0x10)=0x%x\n", __readgsbyte(0x10));
        __writegsbyte(0x10, old);
        printf("test gs  __readgsbyte(0x10)=0x%x\n", __readgsbyte(0x10));
    }

    {
        auto eflags = __readeflags();
        printf("eflags 997 =%p\n", eflags);
    }

    // test __readgsword
    {
        auto old = __readgsword(0x10);
        printf("test gs  __readgsword(0x10)=0x%x\n", old);
        __writegsword(0x10, 0xcccc);
        __incgsword(0x10);
        __addgsword(0x10, 1);
        printf("test gs  __readgsword(0x10)=0x%x\n", __readgsword(0x10));
        __writegsword(0x10, old);
        printf("test gs  __readgsword(0x10)=0x%x\n", __readgsword(0x10));
    }

    {
        auto eflags = __readeflags();
        printf("eflags 998 =%p\n", eflags);
    }

    // test __readgsdword
    {
        auto old = __readgsdword(0x10);
        printf("test gs  __readgsdword(0x10)=0x%x\n", old);
        __writegsdword(0x10, 0xccccdddd);
        __incgsdword(0x10);
        __addgsdword(0x10, 1);
        printf("test gs  __readgsdword(0x10)=0x%x\n", __readgsdword(0x10));
        __writegsdword(0x10, old);
        printf("test gs  __readgsdword(0x10)=0x%x\n", __readgsdword(0x10));
    }

    {
        auto eflags = __readeflags();
        printf("eflags 999 =%p\n", eflags);
    }

    // test __readgsqword
    {
        auto old = __readgsqword(0x10);
        printf("test gs  __readgsqword(0x10)=0x%llx\n", old);
        __writegsqword(0x10, 0x1122334411223344);
        __incgsqword(0x10);
        __addgsqword(0x10, 1);
        printf("test gs  __readgsqword(0x10)=0x%llx\n", __readgsqword(0x10));
        __writegsqword(0x10, old);
        printf("test gs  __readgsqword(0x10)=0x%llx\n", __readgsqword(0x10));
    }

    {
        auto eflags = __readeflags();
        printf("eflags 1010 =%p\n", eflags);
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
    Testintrin();

    return STATUS_SUCCESS;
}
