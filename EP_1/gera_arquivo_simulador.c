#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    srand(2);

    FILE* ptr;
    ptr = fopen("trace.txt", "w+");
    
    int contador = atoi(argv[1]);

    for(int i = 0; i < contador; i++) {
        int t0 = i + 1;
        int dt = rand()%1000;
        long int deadline = ((long int)rand())*((long int)1000) + (long int)t0 + (long int)dt;    
        fprintf(ptr, "processo%d %d %d %ld\n", i, t0, dt, deadline);
    }
    fclose(ptr);

    return 0;
}
