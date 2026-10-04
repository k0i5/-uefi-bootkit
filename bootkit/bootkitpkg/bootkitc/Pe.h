#ifndef BOOTKIT_PE_H
#define BOOTKIT_PE_H

#include <Uefi.h>

UINT8 *PeSection(VOID *Base, CONST CHAR8 *Name);
UINT8 *PePattern(VOID *Base, UINTN Size, CONST UINT8 *Pattern, CONST UINT8 *Mask, UINTN Len);
UINTN PeImageSize(VOID *Base);
UINT64 PeExport(VOID *Base, CONST CHAR8 *Name);

#endif