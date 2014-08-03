#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "zip.h"

int main(int argc, char *argv[])
{
    int i;
    char *file_name = "out.zip";
    FILE *output;

    output = fopen(file_name, "wb");

    for (i = 1; i < argc; i++) {
        printf("[%d] %s\n", i, argv[i]);
        addZip(output, argv[i]);
    }

    if(fclose(output) == -1) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    return 0;
}