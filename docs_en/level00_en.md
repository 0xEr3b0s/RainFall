# RainFall - Level 0

## Passkey for level0
level0

## Passkey for level1
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a

## Objective

Exploit the `level0` binary (setuid level1) in order to obtain a shell and read the file `/home/user/level1/.pass`.

---

## Binary Analysis

### Disassembly of `main`

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

### Key Points

- The program takes a command-line argument
- It converts that argument with `atoi`
- It compares the result against **423** (`0x1a7`)
- If the value differs -> it prints `"No !"` and exits
- If the value is **423** -> privilege elevation + execution of `/bin/sh`

### The Catch

There is no classical memory vulnerability here.
It is simply a matter of understanding the condition and supplying the correct value.

---

## Solution

Run the binary with the argument `423`:

```bash
./level0 423
```

The program enters the `if`, elevates its privileges, and spawns a shell as `level1`.

Once the shell is obtained:

```bash
id
cat /home/user/level1/.pass
```

---

## Reconstructed Source

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

## Summary

| Step | Description |
|------|-------------|
| 1 | Analysis of the disassembly |
| 2 | Identification of the comparison against `0x1a7` (423) |
| 3 | Running `./level0 423` |
| 4 | Shell obtained as level1 |

Technique: **understanding the program's logic** (no memory exploitation).
