# uefi-bootkit

Base de bootkit UEFI. Um DXE driver que carrega antes do sistema operacional e hooka duas funções do firmware: `ExitBootServices` e `LoadImage`.

`ExitBootServices` é a última coisa que o bootloader chama antes de entregar o controle pro kernel nesse momento a memória está toda mapeada, sem proteção de página, e o kernel está carregado mas ainda não rodou. É a janela onde um bootkit age.

Quando o hook dispara, o driver varre a memória física procurando o `ntoskrnl.exe`, acha a base dele, resolve o endereço de `PsInitialSystemProcess` e procura padrões de código conhecidos. Tudo isso fica registrado na saída serial.

Build: `./build.sh` (EDK2) · Run: `./run.sh` (QEMU + OVMF)
