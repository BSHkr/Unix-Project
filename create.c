#include "mytest.h"
#include <stdio.h>
#include <stdlib.h>
#define FILE_COUNT 4
#define ARRAY_SIZE 1024*1024

int main() {
    int i;
    FILE *fp;
    int A[ARRAY_SIZE];

    fp = fopen("datafile.dat", "w");
    
    for (i = 1; i <= ARRAY_SIZE; ++i) {
        fprintf(fp, "%d ", i);
    }

    fprintf(fp, "\n");
    fclose(fp);

    return 0;
}
