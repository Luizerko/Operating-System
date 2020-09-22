#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "processo.h"
#include "time.h"

void* thread(void * arg) {
    Processo* processo = (Processo*)arg;
    while(processo->dt > 0)
        processo->dt--;
    printf("terminei o processo %s\n", processo->nome);
    pthread_exit(NULL);

}

int main (int argc, char* argv[]) {

    FILE* ptr = fopen("trace.txt", "r+");

    FILE* ptr2 = fopen("simulacao.txt", "w+");

    pthread_t t1, t2, t3, t4;

    //int tempo_thread[4] = ([4] -1);


    if (atoi(argv[1]) == 1) {
        queueInit();
        Processo* processo;
        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));

            queueInsert(processo);
            
        }
        long long int tam = queueSize();
        long int ini_1 = 1, ini_2 = 2, ini_3 = 3, ini_4 = 4;
        
        Processo *processo1, *processo2, *processo3, *processo4;
        if(!queueEmpty()) {
            processo1 = queueRemove();
            ini_1 += processo1->dt;
            if(!pthread_create(&t1, NULL, thread, processo1))
                printf("Dei erro 1!\n");
        }
        if(!queueEmpty()) {
            processo2 = queueRemove();
            ini_2 += processo2->dt;
            if(pthread_create(&t2, NULL, thread, processo2))
                printf("Dei erro 2!\n");
        }
        if(!queueEmpty()) {
            processo3 = queueRemove();
            ini_3 += processo3->dt;
            if(pthread_create(&t3, NULL, thread, processo3))
                printf("Dei erro 3!\n");
        }
        if(!queueEmpty()) {
            processo4 = queueRemove();
            ini_4 += processo4->dt;
            if(pthread_create(&t4, NULL, thread, processo4))
                printf("Dei erro 4!\n");
        }

        int a;

        while(!queueEmpty()) {
            //implementar esse bonitao
            if(!queueEmpty() && processo1->dt == 0) {
                fprintf(ptr2, "%s %ld %ld\n", processo1->nome, ini_1, ini_1 - (long int)processo1->t0);
                processo1 = queueRemove();
                ini_1 += processo1->dt;
                if((a = pthread_create(&t1, NULL, thread, processo1)))
                    printf("Dei erro 1! Erro número: %d!\n", a);
            }
            if(!queueEmpty() && processo2->dt == 0) {
                fprintf(ptr2, "%s %ld %ld\n", processo2->nome, ini_2, ini_2 - (long int)processo2->t0);
                processo2 = queueRemove();
                ini_2 += processo2->dt;
                if((a = pthread_create(&t2, NULL, thread, processo2)))
                    printf("Dei erro 2! Erro número: %d!\n", a);
            }
            if(!queueEmpty() && processo3->dt == 0) {
                fprintf(ptr2, "%s %ld %ld\n", processo3->nome, ini_3, ini_3 - (long int)processo3->t0);
                processo3 = queueRemove();
                ini_3 += processo3->dt;
                if((a = pthread_create(&t3, NULL, thread, processo3)))
                    printf("Dei erro 3! Erro número: %d!\n", a);
            }
            if(!queueEmpty() && processo4->dt == 0) {
                fprintf(ptr2, "%s %ld %ld\n", processo4->nome, ini_4, ini_4 - (long int)processo4->t0);
                processo4 = queueRemove();
                ini_4 += processo4->dt;
                if((a = pthread_create(&t4, NULL, thread, processo4)))
                    printf("Dei erro 4! Erro número: %d!\n", a);
            }
        }
        
        fprintf(ptr2, "%lld\n", tam);

    } 
    else if (atoi(argv[1]) == 2) {
        
    } 
    else {
        
    }

    fclose(ptr);
    fclose(ptr2);
    
    return 0;
}