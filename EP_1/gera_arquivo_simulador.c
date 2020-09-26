#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    srand(2);

    FILE* ptr;
    ptr = fopen("trace.txt", "w+");
    
    int limite = atoi(argv[1]);
    int i = 1;
    
    for(int auxiliar_contagem = 1; auxiliar_contagem <= limite; auxiliar_contagem++) {
        int t0 = i;
        int dt = rand()%10 + 1;
        long int deadline = (long int)(rand()%10) + (long int)t0 + (long int)dt;    
        fprintf(ptr, "processo%d %d %d %ld\n", auxiliar_contagem-1, t0, dt, deadline);
        i += rand()%5 + 1;
    }
    fclose(ptr);

    return 0;
}
