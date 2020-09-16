#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "processo.h"


int main (int argc, char* argv[]) {

    int contador = 0;    

    FILE* ptr = fopen("trace.txt", "r+");

    FILE* ptr2 = fopen("simulacao.txt", "w+");

    Processo* processo = malloc(sizeof(Processo));

    if (atoi(argv[1]) == 1) {

        queueInit();
        Processo* processo_atual;
        while(!feof(ptr)) {
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            if(processo_atual != NULL && processo_atual->dt == 0 && !queueEmpty()) {
                fprintf(ptr2, "%s %d %d\n", processo_atual->nome, contador, contador-(processo_atual->t0));
                processo_atual = queueRemove();
            }
            queueInsert(processo);
            printf("%s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);

            contador++;
            
        }
        while(!queueEmpty()) {
            //implementar esse bonitao
        }
        
    } 
    else if (atoi(argv[1]) == 2) {
        
    } 
    else {
        
    }

    fclose(ptr);
    return 0;
}



