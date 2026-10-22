# Level0

---

# 1

execute the program:

```asm
./level0
Segmentation fault

./level0 test
No !

./level0 121231231
No !

```

# 2

using objdump -M intel -d level0 | grep -A60 "<main>:"

It shows us that in the upper part of the asm code the program is calling to atoi.
So looking at the first part you can clearly see that you need to give a value in entry of the program
and then this entry is sended to atoi.

# 3

Then here we seeing that there is a comparaison between the exit value of atoi and 0x1a7 ( 423 )

```asm
cmp    eax,0x1a7
```

# 4

and then once you put 423 into you program parameters it give u access to a shell.
The rest of the asm code is just noise to lurr you.

