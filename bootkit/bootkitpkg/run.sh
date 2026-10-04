#!/bin/bash
set -e

EFI="$(pwd)/Build/BootkitPkg/DEBUG_GCC5/X64/Bootkit.efi"

if [ ! -f "$EFI" ]; then
    echo "build first"
    exit 1
fi

OVMF_CODE=$(find /usr/share -name 'OVMF_CODE*.fd' 2>/dev/null | head -1)
OVMF_VARS=$(find /usr/share -name 'OVMF_VARS*.fd' 2>/dev/null | head -1)

if [ -z "$OVMF_CODE" ]; then
    echo "install ovmf"
    exit 1
fi

mkdir -p Build/esp/EFI/BOOT
cp "$EFI" Build/esp/EFI/BOOT/BOOTX64.EFI

if [ ! -f Build/OVMF_VARS.fd ]; then
    cp "$OVMF_VARS" Build/OVMF_VARS.fd
fi

dd if=/dev/zero of=Build/esp.img bs=1M count=64 status=none
mkfs.vfat -F 32 Build/esp.img >/dev/null
mcopy -i Build/esp.img -s Build/esp/* ::

qemu-system-x86_64 \
    -M q35 \
    -m 2048 \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file=Build/OVMF_VARS.fd \
    -drive file=Build/esp.img,format=raw \
    -serial stdio \
    -display none