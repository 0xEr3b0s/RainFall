#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

static void p(void)
{
	uint unaff_retaddr;
	char local_50 [76];

	fflush(stdout);
	gets(local_50);
	if ((unaff_retaddr & 0xb0000000) == 0xb0000000) {
		printf("(%p)\n", &unaff_retaddr);
		exit(1);
	}
	puts(local_50);
	strdup(local_50);
	return;
}


int main(void)
{
	p();
	return 0;
}
