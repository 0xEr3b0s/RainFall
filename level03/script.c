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

find the address of m:

objdump -D level3 | grep <m>

Result:
0804988c <m>:

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
#include <stdio.h>
#include <stdlib.h>

int m = 0; // -> variable globale address probable: 0x0804988c


void frame_dummy(void) {
	return;
}

void v(void) {
	char buffer[520];	//0xbffff520:     0x00000000
	fgets(buffer, 512, stdin);
	printf(buffer);
	if (m == 64) {
		fwrite("Wait what ?!\n", 1, 12, stdout);
		system("/bin/sh");
	}
	return ;
}

int main(void) { // address de debut: 0x804851a
	v();
	return 0;
}
