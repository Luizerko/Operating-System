#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
#include "processo.h"
#include "time.h"

pthread_t t1, t2, t3, t4;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long int tempo = 1;

void* thread(void * arg) {
    Processo* processo = (Processo*)arg;
    while(processo->dt > 0) {
        processo->dt--;
        tempo++;
        sleep(1);
    }
    printf("terminei o processo %s\n", processo->nome);
    pthread_exit(NULL);
}

void sink(long int indice, Processo* processos[]) {
    Processo* aux = processos[1];
    long int pai = 1;
    while(2*pai <= indice-1) {
        if(processos[pai*2]->dt < processos[(pai*2)+1]->dt) {
            if(processos[pai*2]->dt < aux->dt) {
                processos[pai] = processos[pai*2];
                pai = 2*pai;
            }
            else break;
        }
        else {
            if(processos[(pai*2) + 1]->dt < aux->dt) {
                processos[pai] = processos[(pai*2) + 1];
                pai = (2*pai) + 1;
            }
            else break;
        }
    }
    processos[pai] = aux;
}

void swim (long int filho, Processo* processos[]) {
    long int pai = filho/2;
    Processo* aux = processos[filho];
    while (filho > 1 && processos[pai]->dt > processos[filho]->dt) {
        processos[filho] = processos[pai];
        filho = pai;
        pai = filho/2;
    }
    processos[pai] = aux;
}

void heap_insert(Processo** heap_minimo, long int indice, Processo* processo) {
    heap_minimo[indice] = processo;
    swim(indice, heap_minimo);
}

//Quando o índice == 1, o heap está vazio 
void heap_remove(Processo** heap_minimo, long int indice) {
    heap_minimo[1] = heap_minimo[indice-1];
    sink(indice, heap_minimo);
}

int heap_empty(long int indice) {
    if (indice == 1)
        return 1;
    return 0;
}

void* thread2_1(void * arg) {
    Processo* processo = (Processo*)arg;
    while(processo->dt > 0) {
        pthread_mutex_lock(&mutex);
        processo->dt--;
        tempo++;
        sleep(1);
        pthread_mutex_unlock(&mutex);
    }
    printf("terminei o processo %s\n", processo->nome);
    pthread_exit(NULL);
}

void* thread2_2(void* arg) {
    Processo** heap_minimo = (Processo**)arg;
    Processo* processo;
    long int indice = 1;
    while(heap_minimo != NULL) {
        pthread_mutex_lock(&mutex);
        processo = queueTop();
        Processo* aux = heap_minimo[1];
        if(processo != NULL && processo->t0 <= tempo) {
            queueRemove();
            if(!heap_empty(indice) && aux->dt == 0) {
                heap_remove(heap_minimo, indice);
                indice--;
            }
            heap_insert(heap_minimo, indice, processo); //com swin
            indice++;
            if(heap_minimo[1] != aux) {
                if(indice != 2 && aux->dt != 0)
                    pthread_cancel(t1); //qualquer coisa bota o join pra ver se rola.
                pthread_create(&t1, NULL, thread2_1, heap_minimo[1]);
            }
        }
        else {
            if(aux->dt == 0){
                heap_remove(heap_minimo, indice);
                indice--;
            }
            if(!heap_empty(indice))
                pthread_create(&t1, NULL, thread2_1, heap_minimo[1]);
        }
        if (queueEmpty() && heap_empty(indice)) {
            free(heap_minimo);
            heap_minimo = NULL;
        }
        sleep(1);
        tempo++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main (int argc, char* argv[]) {

    FILE* ptr = fopen("trace.txt", "r+");

    FILE* ptr2 = fopen("simulacao.txt", "w+");

    //int tempo_thread[4] = ([4] -1);


    if (atoi(argv[1]) == 1) {
        queueInit();
        Processo* processo;
        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            if(strcmp(processo->nome, ""))
                queueInsert(processo);
            
        }
        long long int tam = queueSize();
        long int ini_1 = 1;
        //long int ini_2 = 2, ini_3 = 3, ini_4 = 4;
        int a;
        Processo *processo1;
        //Processo *processo2, *processo3, *processo4;
        if(!queueEmpty()) {
            processo1 = queueRemove();
            ini_1 += processo1->dt;
            if(pthread_create(&t1, NULL, thread, processo1))
                printf("Dei erro 1!\n");
        }
        
        /*
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
        }*/

        while(!queueEmpty()) {
            //implementar esse bonitao
            
            pthread_join(t1, NULL);
            if(!queueEmpty() && processo1->dt == 0 && queueTop()->t0 <= tempo) {
                fprintf(ptr2, "%s %ld %ld\n", processo1->nome, ini_1, ini_1 - (long int)processo1->t0);
                processo1 = queueRemove();
                ini_1 += processo1->dt;
                if((a = pthread_create(&t1, NULL, thread, processo1)))
                    printf("Dei erro 1! Erro número: %d!\n", a);
            }
            else {
                sleep(1);
                tempo++;
            }
            /*
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
            */
        }

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        pthread_join(t3, NULL);
        pthread_join(t4, NULL);

        fprintf(ptr2, "%lld\n", tam);

    } 
    else if (atoi(argv[1]) == 2) {
        queueInit();
        Processo* processo;
        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));

            queueInsert(processo);
            
        }
        
        Processo* heap_minimo[queueSize() + 1];
        
        int a;
        
        if((a = pthread_create(&t2, NULL, thread2_2, heap_minimo[1])))
            printf("Dei erro! Erro númeor: %d!\n", a);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

    } 
    else {
        
    }

    fclose(ptr);
    fclose(ptr2);
    
    return 0;
}