#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    srand(2);

    FILE* ptr;
    ptr = fopen("trace.txt", "w+");
    
    int contador = atoi(argv[1]);

    for(int i = 0; i < contador; i++) {
        int t0 = rand()%1000;
        int dt =rand()%1000;
        int deadline = rand()%1000 + t0 + dt;    
        fprintf(ptr, "processo%d %d %d %d\n", i, t0, dt, deadline);
    }
    fclose(ptr);

    return 0;
}
