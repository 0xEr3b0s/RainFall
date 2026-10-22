#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int ac, char *av[]) {

	int atoied = atoi(av[1]);
	char **shell = {0};
	unsigned int uid;
	unsigned int gid;

	if (atoied == 0x1a7) {
		shell[0] = "/bin/sh";
		shell[1] = 0;
		uid = geteuid();
		gid = getegid();

		setreuid(uid, uid);
		setregid(gid, gid);
		execv("/bin/sh", shell);
	} else {
		fwrite("No !\n", 1, 5, stderr);
	}
	return 0;
}
