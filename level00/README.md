# Level 00 — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `level0` |
| **User** | `level0` |
| **Goal user** | `level1` |
| **Protections** | *(no memory vulnerability at play; logic-based level)* |
| **Password (level0)** | `level0` |
| **Password (level1)** | [password.txt](password.txt) |

**Objective:** exploit the `level0` binary (setuid `level1`) so as to obtain a shell and read `/home/user/level1/.pass`.

---

## 1. Reconnaissance

The binary receives, upon invocation, a single command-line argument. No environment variable nor stdin input is solicited. Its behaviour bifurcates entirely upon the value of that argument, once converted.

---

## 2. Static Analysis

Disassembly of `main` reveals the following sequence:

```asm
0x08048ed4 <+20>:   call   0x8049710 <atoi>
0x08048ed9 <+25>:   cmp    $0x1a7,%eax          ; 0x1a7 = 423
0x08048ede <+30>:   jne    0x8048f58 <main+152> ; if different -> "No !"
...
; if equal to 423:
call   strdup("/bin/sh")
call   getegid / geteuid
call   setresgid / setresuid
call   execv("/bin/sh", ...)
```

- The program takes a command-line argument and converts it by way of `atoi`.
- The resulting value is compared against **423** (`0x1a7`).
- Should the values differ, `"No !"` is printed and the program exits.
- Should the values coincide, the process elevates its privileges and executes `/bin/sh`.

There is no classical memory vulnerability herein — the matter is one of comprehending the condition and furnishing the correct value.

A reconstruction of the source, for the sake of clarity:

[source.c](source.c)

---

## 3. Dynamic Analysis

None for this level.

---

## 4. Vulnerability

No vulnerability just a logic takeover.

---

## 5. Exploitation


```bash
./level0 423
```

The program thereupon enters the favourable branch, elevates its privileges, and spawns a shell as `level1`.

---

## 6. Result

Once the shell is obtained:

```bash
id
cat /home/user/level1/.pass
```

| Step | Description |
|------|-------------|
| 1 | Analysis of the disassembly |
| 2 | Identification of the comparison against `0x1a7` (423) |
| 3 | Running `./level0 423` |
| 4 | Shell obtained as `level1` |

---

## Notes & Lessons

This level, wanting any memory corruption, serves chiefly as an introduction to disassembly reading: the discipline of following a condition through to its consequence, rather than the more familiar art of overflow.
