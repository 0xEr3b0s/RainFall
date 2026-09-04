# Level 00 — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `level0` |
| **User** | `level0` |
| **Goal user** | `level1` |
| **Protections** | *(no memory vulnerability at play; logic-based level)* |
| **Password (current level)** | `level0` |
| **Password (obtained)** | `1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a` |

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

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    if (argc < 2 || atoi(argv[1]) != 423)
    {
        fwrite("No !\n", 1, 5, stderr);
        return 0;
    }

    char *shell = strdup("/bin/sh");

    gid_t egid = getegid();
    uid_t euid = geteuid();

    setresgid(egid, egid, egid);
    setresuid(euid, euid, euid);

    char *args[] = { shell, NULL };
    execv(shell, args);

    return 0;
}
```

---

## 3. Dynamic Analysis

Superfluous at this level: the condition, once identified through static reading alone, suffices to determine the correct course of action. No breakpoint or runtime observation was required.

---

## 4. Vulnerability

Properly speaking, no vulnerability is present — the level tests one's capacity to read disassembly and grasp a program's logic, rather than any flaw in memory handling.

---

## 5. Exploitation

The strategy consists simply in supplying the value the comparison expects:

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
