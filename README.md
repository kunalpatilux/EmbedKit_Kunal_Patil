# EmbedKit_Kunal Patil

**Assessment:** Embedded Developer Assessment — Fresher
**Company:** Embed Square Solutions Pvt. Ltd.

---

## Modules

| File | Description |
|------|-------------|
| `ringbuf.c` | A fixed-capacity 8-byte circular (ring) buffer for `uint8_t` data, implementing interrupt-safe FIFO queue operations used in embedded UART firmware. |

---

## Build Instructions

### Requirements
- GCC compiler (C99 support)
- Linux, macOS, or Windows (MinGW / WSL)
- No external libraries required

### Step 1 — Clone the Repository

```bash
git clone https://github.com/kunalpatilux/EmbedKit_Kunal_Patil.git
cd EmbedKit_Kunal_Patil
```

### Step 2 — Compile

```bash
gcc -Wall -std=c99 ringbuf.c -o ringbuf
```

### Step 3 — Run

```bash
./ringbuf
```

---

## Expected Output

```
=== Embed Kit: Ring Buffer Demo ===

Step 1: Write 8 bytes 
[WRITE] 0x41 -> OK (count=1)
[WRITE] 0x42 -> OK (count=2)
[WRITE] 0x43 -> OK (count=3)
[WRITE] 0x44 -> OK (count=4)
[WRITE] 0x45 -> OK (count=5)
[WRITE] 0x46 -> OK (count=6)
[WRITE] 0x47 -> OK (count=7)
[WRITE] 0x48 -> OK (count=8) FULL

Step 2: Write to full buffer 
[WRITE] 0x99 -> FAIL (buffer full)

   Step 3: Read 3 bytes 
[READ]  -> 0x41 (count=7)
[READ]  -> 0x42 (count=6)
[READ]  -> 0x43 (count=5)
Count after 3 reads: 5 (expected 5)

Step 4: Write 3 new bytes (0x49, 0x4A, 0x4B) 
[WRITE] 0x49 -> OK (count=6)
[WRITE] 0x4A -> OK (count=7)
[WRITE] 0x4B -> OK (count=8) FULL

 Step 5: Read all 8 remaining bytes 
[READ]  -> 0x44 (count=7)
[READ]  -> 0x45 (count=6)
[READ]  -> 0x46 (count=5)
[READ]  -> 0x47 (count=4)
[READ]  -> 0x48 (count=3)
[READ]  -> 0x49 (count=2)
[READ]  -> 0x4A (count=1)
[READ]  -> 0x4B (count=0) EMPTY

 Step 6: Read from empty buffer 
[READ]  (empty) -> FAIL (buffer empty)

=== Demo Complete ===
```

---

## Project Structure

```
EmbedKit_Kunal_Patil/
├── ringbuf.c      // Ring buffer implementation + demo
└── README.md      // This file
```

---

## Implementation Notes

- All data types use `uint8_t` / `int8_t` from `<stdint.h>` — no plain `int` or `char`
- All constants defined with `#define` — no magic numbers in logic
- Only standard C libraries used: `<stdio.h>`, `<stdint.h>`, `<string.h>`, `<stdlib.h>`
- **Bonus implemented:** Head/tail wrap-around uses `& (BUFFER_SIZE - 1)` bitwise AND instead of `% BUFFER_SIZE` modulo, which avoids a costly division instruction on MCUs without a hardware divider — valid only because `BUFFER_SIZE = 8` is a power of 2
- Compiles with zero errors and zero warnings under `gcc -Wall -std=c99`
