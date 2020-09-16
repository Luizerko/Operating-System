#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct processo {
    char nome[30];
    int t0; 
    int dt; 
    int deadline;
    int tf;
    int tr;
} Processo;


int main () {
    
    FILE* ptr;
    ptr = fopen("trace.txt", "r+");
    Processo* processo = malloc(sizeof(Processo));
    while(!feof(ptr)) {
        fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
        printf("%s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
    }

    fclose(ptr);
    return 0;
}



