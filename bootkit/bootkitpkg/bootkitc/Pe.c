#include "Pe.h"
#include <Library/BaseMemoryLib.h>

#define DOS_MAGIC 0x5A4D
#define NT_MAGIC  0x00004550
#define PE64     0x20B

typedef struct {
    UINT16 e_magic;
    UINT8  _pad[58];
    UINT32 e_lfanew;
} DOS_HDR;

typedef struct {
    UINT32 Signature;
    UINT16 Machine;
    UINT16 NumberOfSections;
    UINT32 TimeDateStamp;
    UINT32 PointerToSymbolTable;
    UINT32 NumberOfSymbols;
    UINT16 SizeOfOptionalHeader;
    UINT16 Characteristics;
} FILE_HDR;

typedef struct {
    UINT16 Magic;
    UINT8  _pad0[54];
    UINT32 SizeOfImage;
    UINT8  _pad1[0x70 - 0x3C];
    UINT32 NumberOfRvaAndSizes;
} OPT_HDR64_HEAD;

typedef struct {
    UINT8  Name[8];
    UINT32 VirtualSize;
    UINT32 VirtualAddress;
    UINT32 SizeOfRawData;
    UINT32 PointerToRawData;
    UINT8  _pad[16];
} SEC_HDR;

typedef struct {
    UINT32 VirtualAddress;
    UINT32 Size;
} DATA_DIR;

static DOS_HDR *dos(VOID *B) { return (DOS_HDR *)B; }
static FILE_HDR *fh(VOID *B) { return (FILE_HDR *)((UINT8 *)B + dos(B)->e_lfanew + 4); }
static UINT8 *oh(VOID *B) { return (UINT8 *)B + dos(B)->e_lfanew + 4 + sizeof(FILE_HDR); }
static SEC_HDR *sec(VOID *B) { return (SEC_HDR *)(oh(B) + fh(B)->SizeOfOptionalHeader); }

UINT8 *PeSection(VOID *Base, CONST CHAR8 *Name) {
    if (dos(Base)->e_magic != DOS_MAGIC) return NULL;
    if (*(UINT32 *)((UINT8 *)Base + dos(Base)->e_lfanew) != NT_MAGIC) return NULL;
    SEC_HDR *S = sec(Base);
    for (UINTN i = 0; i < fh(Base)->NumberOfSections; i++) {
        if (CompareMem(S[i].Name, Name, AsciiStrLen(Name)) == 0) {
            return (UINT8 *)Base + S[i].VirtualAddress;
        }
    }
    return NULL;
}

UINTN PeImageSize(VOID *Base) {
    if (dos(Base)->e_magic != DOS_MAGIC) return 0;
    if (*(UINT32 *)((UINT8 *)Base + dos(Base)->e_lfanew) != NT_MAGIC) return 0;
    OPT_HDR64_HEAD *O = (OPT_HDR64_HEAD *)oh(Base);
    if (O->Magic != PE64) return 0;
    return O->SizeOfImage;
}

UINT8 *PePattern(VOID *Base, UINTN Size, CONST UINT8 *Pattern, CONST UINT8 *Mask, UINTN Len) {
    UINT8 *P = (UINT8 *)Base;
    for (UINTN i = 0; i + Len <= Size; i++) {
        UINTN J = 0;
        for (; J < Len; J++) {
            if (Mask && Mask[J] == 0) continue;
            if (P[i + J] != Pattern[J]) break;
        }
        if (J == Len) return P + i;
    }
    return NULL;
}

UINT64 PeExport(VOID *Base, CONST CHAR8 *Name) {
    if (dos(Base)->e_magic != DOS_MAGIC) return 0;
    if (*(UINT32 *)((UINT8 *)Base + dos(Base)->e_lfanew) != NT_MAGIC) return 0;

    OPT_HDR64_HEAD *O = (OPT_HDR64_HEAD *)oh(Base);
    if (O->Magic != PE64) return 0;
    if (O->NumberOfRvaAndSizes < 1) return 0;

    DATA_DIR *Dirs = (DATA_DIR *)((UINT8 *)O + 0x70);
    DATA_DIR *Exp = &Dirs[0];
    if (Exp->VirtualAddress == 0) return 0;

    UINT8 *Dir = (UINT8 *)Base + Exp->VirtualAddress;
    UINT32 *Names = (UINT32 *)(Dir + 0x20);
    UINT16 *Ords = (UINT16 *)(Dir + 0x24);
    UINT32 *Funcs = (UINT32 *)(Dir + 0x1C);
    UINT32 Count = *(UINT32 *)(Dir + 0x18);

    for (UINT32 i = 0; i < Count; i++) {
        CHAR8 *N = (CHAR8 *)Base + Names[i];
        if (AsciiStrCmp(N, Name) == 0) {
            return (UINT64)((UINT8 *)Base + Funcs[Ords[i]]);
        }
    }
    return 0;
}