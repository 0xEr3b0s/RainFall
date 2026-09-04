#include <stdio.h>
#include <stdlib.h>

int m = 0;

void p(char *buffer) {
	printf(buffer);
	return ;
}

void n(void) {
	char buffer[520];

	fgets(buffer, 512, stdin);
	p(buffer);
	if (m == 0x1025544) {
		system("cat /home/user/level4/.pass");
	}
	return ;
}

int main(void) {
	n();
	return 0;
}
