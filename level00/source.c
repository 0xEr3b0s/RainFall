#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    if (argc < 2 || atoi(argv[1]) != 423)
    {
        fwrite("No !\n", 1, 5, stderr);
        return 0;
    }

    char *shell = strdup("/bin/sh");

    gid_t egid = getegid();
    uid_t euid = geteuid();

    setresgid(egid, egid, egid);
    setresuid(euid, euid, euid);

    char *args[] = { shell, NULL };
    execv(shell, args);

    return 0;
}
