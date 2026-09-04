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
