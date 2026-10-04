#!/bin/bash
set -e

EDK2="${EDK2:-$HOME/edk2}"
PKG="$(pwd)/BootkitPkg"

if [ ! -d "$EDK2" ]; then
    echo "clone edk2 first:"
    echo "  git clone --recursive https://github.com/tianocore/edk2.git $EDK2"
    exit 1
fi

cd "$EDK2"
source edksetup.sh
make -C BaseTools -j"$(nproc)"

if [ ! -d "$EDK2/BootkitPkg" ]; then
    ln -sf "$PKG" "$EDK2/BootkitPkg"
fi

export PACKAGES_PATH="$EDK2:$EDK2/BootkitPkg"

build -a X64 -t GCC5 -p BootkitPkg/BootkitPkg.dsc -b DEBUG