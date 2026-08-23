#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run(void)
{
    write(1, "Good... oh wait!\n", 17);
    execl("/bin/sh", "sh", NULL);
}

int main(void)
{
    char buff[76];

    gets(buff);
    return 0;
}
