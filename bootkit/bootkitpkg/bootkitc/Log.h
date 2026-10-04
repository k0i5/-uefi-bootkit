#ifndef BOOTKIT_LOG_H
#define BOOTKIT_LOG_H

#include <Uefi.h>

VOID LogInit(VOID);
VOID LogStr(CONST CHAR8 *S);
VOID LogHex(UINT64 V);
VOID LogU64(UINT64 V);

#endif