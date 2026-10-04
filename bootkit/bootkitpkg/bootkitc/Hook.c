#include "Hook.h"
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#define STUB_SIZE 12
#define TRAMP_SIZE 32

static UINT8 stub[STUB_SIZE] = {
    0x48, 0xB8,                         /* mov rax, imm64 */
    0, 0, 0, 0, 0, 0, 0, 0,
    0xFF, 0xE0                          /* jmp rax */
};

EFI_STATUS InlineHook(VOID *Target, VOID *Detour, VOID **Trampoline) {
    if (!Target || !Detour) return EFI_INVALID_PARAMETER;

    UINT8 *Tramp = AllocatePool(TRAMP_SIZE);
    if (!Tramp) return EFI_OUT_OF_RESOURCES;

    CopyMem(Tramp, Target, STUB_SIZE);

    UINT8 JmpBack[STUB_SIZE];
    CopyMem(JmpBack, stub, STUB_SIZE);
    UINT64 Ret = (UINT64)Target + STUB_SIZE;
    CopyMem(&JmpBack[2], &Ret, 8);
    CopyMem(Tramp + STUB_SIZE, JmpBack, STUB_SIZE);

    UINT8 NewStub[STUB_SIZE];
    CopyMem(NewStub, stub, STUB_SIZE);
    UINT64 Dest = (UINT64)Detour;
    CopyMem(&NewStub[2], &Dest, 8);

    CopyMem(Target, NewStub, STUB_SIZE);

    if (Trampoline) *Trampoline = Tramp;
    return EFI_SUCCESS;
}

EFI_STATUS InlineUnhook(VOID *Target) {
    if (!Target) return EFI_INVALID_PARAMETER;
    return EFI_SUCCESS;
}