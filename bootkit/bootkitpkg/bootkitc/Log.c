#include "Log.h"
#include <Library/SerialPortLib.h>
#include <Library/BaseLib.h>

VOID LogInit(VOID) {
    SerialPortInitialize();
}

VOID LogStr(CONST CHAR8 *S) {
    if (!S) return;
    SerialPortWrite((UINT8 *)S, AsciiStrLen(S));
}

VOID LogHex(UINT64 V) {
    CHAR8 B[19];
    B[0] = '0'; B[1] = 'x';
    for (INTN i = 15; i >= 0; i--) {
        UINT8 N = (UINT8)((V >> (i * 4)) & 0xF);
        B[2 + (15 - i)] = (CHAR8)(N < 10 ? '0' + N : 'a' + (N - 10));
    }
    B[18] = 0;
    LogStr(B);
}

VOID LogU64(UINT64 V) {
    CHAR8 B[21];
    UINTN I = 20;
    B[I] = 0;
    if (V == 0) { B[--I] = '0'; }
    while (V > 0) {
        B[--I] = (CHAR8)('0' + (V % 10));
        V /= 10;
    }
    LogStr(&B[I]);
}