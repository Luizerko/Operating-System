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
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
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
        if(2*pai+1 > indice-1 || processos[pai*2]->dt < processos[(pai*2)+1]->dt) {
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
    processos[filho] = aux;
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

void* thread2(void * arg) {
    Processo* processo = (Processo*)arg;
    while(processo->dt > 0) {
        printf("Entrei na thread!\n");
        pthread_mutex_lock(&mutex_2);
        processo->dt--;
        tempo++;
        sleep(1);
        if(processo->dt == 0)
            printf("terminei o processo %s\n", processo->nome);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

/*void* thread2_2(void* arg) {
    Processo** heap_minimo = (Processo**)arg;
    Processo* processo;
    long int indice = 1;
    while(heap_minimo != NULL) {
        pthread_mutex_lock(&mutex);
        processo = queueTop();
        Processo* aux = heap_minimo[1];

        if(!heap_empty(indice) && aux->dt == 0) {
            heap_remove(heap_minimo, indice);
            indice--;
        }
        
        if(processo != NULL && processo->t0 <= tempo) {
            while(processo != NULL && processo->t0 <= tempo) {
                queueRemove();
                heap_insert(heap_minimo, indice, processo); //com swin
                indice++;
                processo = queueTop();
            }
            if(heap_minimo[1] != aux) {
                if(indice != 2 && aux->dt != 0)
                    pthread_cancel(t1); //qualquer coisa bota o join pra ver se rola.
                pthread_create(&t1, NULL, thread2_1, heap_minimo[1]);
            }
        }
        else if(!heap_empty(indice) && aux->dt == 0)
            pthread_create(&t1, NULL, thread2_1, heap_minimo[1]);

        if (queueEmpty() && heap_empty(indice)) {
            free(heap_minimo);
            heap_minimo = NULL;
        }

        sleep(1);
        tempo++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
} */

int main (int argc, char* argv[]) {

    FILE* ptr = fopen("trace.txt", "r+");

    FILE* ptr2 = fopen("simulacao.txt", "w+");

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
        int a;
        int flag_escreve = 1;
        Processo *processo1;

        if(!queueEmpty()) {
            processo1 = queueRemove();
            if(pthread_create(&t1, NULL, thread, processo1))
                printf("Dei erro 1!\n");
        }
        
        while(!queueEmpty()) {
            
            pthread_join(t1, NULL);
            if(flag_escreve) {
                fprintf(ptr2, "%s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
                flag_escreve = 0;
            }
            if(!queueEmpty() && processo1->dt == 0 && queueTop()->t0 <= tempo) {
                processo1 = queueRemove();
                flag_escreve = 1;
                if((a = pthread_create(&t1, NULL, thread, processo1)))
                    printf("Dei erro 1! Erro número: %d!\n", a);
            }
            else {
                sleep(1);
                tempo++;
            }
            
        }

        pthread_join(t1, NULL);
        fprintf(ptr2, "%s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
        
        fprintf(ptr2, "%lld\n", tam);

    } 
    else if (atoi(argv[1]) == 2) {
        queueInit();
        tempo = 0;
        Processo* processo;
        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));

            queueInsert(processo);
            
        }
        
        Processo** heap_minimo;
        heap_minimo = malloc((queueSize()+1)*sizeof(Processo*));
        /*int a;
        if((a = pthread_create(&t2, NULL, thread2_2, heap_minimo[1])))
            printf("Dei erro! Erro númeor: %d!\n", a);*/
        pthread_mutex_lock(&mutex_2);

        long int indice = 1;
        while(heap_minimo != NULL) {

            if(heap_empty(indice))
                pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex);
            processo = queueTop();
            Processo* aux = heap_minimo[1];

            if(heap_empty(indice)) {
                tempo++;
                //sleep(1)
            }

            if(!heap_empty(indice) && aux->dt == 0) {
                //printf("terminei um processo!\n");
                fprintf(ptr2, "%s %ld %ld\n", processo->nome, tempo, tempo - (long int)processo->t0);
                heap_remove(heap_minimo, indice);
                indice--;
            }
            
            if(processo != NULL && processo->t0 <= tempo) {
                while(processo != NULL && processo->t0 <= tempo) {
                    printf("%s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
                    queueRemove();
                    heap_insert(heap_minimo, indice, processo); //com swin
                    printf("%s %d %d %d\n", heap_minimo[1]->nome, heap_minimo[1]->t0, heap_minimo[1]->dt, heap_minimo[1]->deadline);
                    indice++;
                    processo = queueTop();
                }
                if(heap_minimo[1] != aux) {
                    if(indice != 2 && aux->dt != 0)
                        pthread_cancel(t1); //qualquer coisa bota o join pra ver se rola.
                    pthread_create(&t1, NULL, thread2, heap_minimo[1]);
                }
            }
            else if(!heap_empty(indice) && aux->dt == 0)
                pthread_create(&t1, NULL, thread2, heap_minimo[1]);

            if (queueEmpty() && heap_empty(indice)) {
                free(heap_minimo);
                heap_minimo = NULL;
            }

            if(!heap_empty(indice))
                pthread_mutex_unlock(&mutex_2);
        }

        pthread_join(t1, NULL);


    } 
    else {
        
    }

    fclose(ptr);
    fclose(ptr2);
    
    return 0;
}