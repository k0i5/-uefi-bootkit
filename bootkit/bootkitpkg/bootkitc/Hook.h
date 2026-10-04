#ifndef BOOTKIT_HOOK_H
#define BOOTKIT_HOOK_H

#include <Uefi.h>

EFI_STATUS InlineHook(VOID *Target, VOID *Detour, VOID **Trampoline);
EFI_STATUS InlineUnhook(VOID *Target);

#endif