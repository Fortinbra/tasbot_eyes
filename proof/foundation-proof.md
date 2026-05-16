# Pico foundation proof

This file captures the reviewed evidence for the isolated `pico_build\` scaffold after the rejection of the earlier scaffold slice. It separates software proof that was reproduced in this workspace from the hardware serial capture that still depends on an attached Pico board.

## Reproduction command

```text
powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1
powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1 -BuildDir C:\ws\tasbot_eyes\pico_build\build\ws2812-proof
```

The published hashes below come from the script's default build directory, `C:\ws\tasbot_eyes\pico_build\build\ws2812-proof`. The bare command and the explicit `-BuildDir` form now rebuild the same path, so they reproduce the same path-sensitive `.elf`, `.dis`, and `.elf.map` hashes.

## Reproduced build evidence

Fresh Pico configure/build completed from `pico_build\build\ws2812-proof` with:

```text
cmake -G Ninja -S C:\ws\tasbot_eyes\pico_build -B C:\ws\tasbot_eyes\pico_build\build\ws2812-proof -DPICO_SDK_PATH=C:\ws\pico-sdk -DPICO_BOARD=pimoroni_plasma2350
cmake --build C:\ws\tasbot_eyes\pico_build\build\ws2812-proof
```

This proof build exercises the Phase 2 hardware seam with the new WS2812B-over-PIO `hw_led` backend compiled into the firmware image.

Artifacts reproduced in that build:

| Artifact | Size (bytes) | SHA-256 |
|---|---:|---|
| `tasbot_eyes_pico.bin` | 49764 | `842185DECF98CAB6738CA1514D8E3FE7571305D636903CD6AF97CE54DF6F896F` |
| `tasbot_eyes_pico.dis` | 514530 | `DE095D5401A89E16A0363ECC22815E371A88BAE9091AE1BEE423A3C7DECB25D0` |
| `tasbot_eyes_pico.elf` | 689740 | `EC7D5F6CCAE3F93D9145AB8D78272AFD8720C9D78C2BE7D90DFCFD90F7927913` |
| `tasbot_eyes_pico.elf.map` | 457002 | `09A75580B21000645723056E983880F9FD965069E4611A1D82928584C833A2BF` |
| `tasbot_eyes_pico.hex` | 140048 | `3BD00D06D250F7BEA0490553B451F956D0255125485D92455DD5DF37AA916F53` |
| `tasbot_eyes_pico.uf2` | 100352 | `80C7924E5FFED719BD3C7E4CD7892011E4E3265903BF53ED503EE152D4A56630` |

The proof build also confirmed the compiled ELF embeds the expected boot/ready/runtime contract strings:

- `tasbot_eyes pico_build booting`
- `tasbot_eyes pico_build ready`
- `board contract: WS2812B on GPIO15, active 8x28 (224 pixels) within physical 8x32 (256 pixels)`
- `runtime seam: logical 28x8 active frame -> column-serpentine mapper -> 256 LED physical transport`

## Root baseline check

The legacy root build was re-run in a fresh Visual Studio build directory. It still fails in the old host-specific way on missing legacy dependencies and POSIX assumptions:

- `gif_lib.h` missing from `main.c`, `gif.c`, and `arguments.c`
- `unistd.h` missing in `led.c`
- `ws2811/ws2811.h` missing from multiple root translation units
- `dirent.h` missing in `filesystem.c`
- `pthread.h` missing in `network.c`

That is useful regression evidence: the `pico_build\` slice did not take over or silently mutate the root build story.

## Ready-window contract

The firmware now makes the board/protocol contract explicit in source:

1. LED protocol: `WS2812B`
2. LED data pin: `GPIO15` (`PLASMA2350_DATA_PIN` / board default path)
3. Active pixel count: `224`
4. Physical chain length: `256`
5. Smoke cadence: `750 ms` per phase (`4/3 Hz`, 4 phases per cycle)

The firmware also keeps the boot-visible contract explicit in source:

1. `stdio_init_all()`
2. immediate boot banner: `tasbot_eyes pico_build booting`
3. board banner: `board contract: WS2812B on GPIO15, active 8x28 (224 pixels) within physical 8x32 (256 pixels)`
4. 2000 ms delay
5. ready banner repeated 4 times at 250 ms intervals
6. embedded `colorful.gif` playback loop begins

That means the first ready line is scheduled at roughly 2.0 s after boot and the ready window stays active until roughly 2.75 s, which keeps the Phase 1 requirement inside the 3-second gate while still being simple enough to review.

## Hardware capture status

No Pico serial device or removable RP2 volume was present in this session, so the full hardware serial-ready gate could not be closed here. The evidence above is therefore honest local proof of build identity plus a reproducible firmware contract, not a claim that the on-device serial gate has already passed.

To close the remaining hardware portion, rerun:

```text
powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1 -SerialPort COMx
```

Acceptance for the hardware slice should require the captured serial transcript to show at least one `tasbot_eyes pico_build ready` line within the first 3 seconds after reset.
