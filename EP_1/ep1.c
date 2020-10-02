#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
#include "processo.h"
#include "time.h"

long int tempo = 1;
long int quantum = 3;
int flag_d = 0;

pthread_t t1;

pthread_t* t_v;
pthread_mutex_t mutex;
pthread_mutex_t* mutex_v;

void* thread(void * arg) {
    Processo* processo = (Processo*)arg;
    if(flag_d)
        fprintf(stderr, "Processo %s usando CPU: %d\n", processo->nome, sched_getcpu());
    while(processo->dt > 0) {
        processo->dt--;
        tempo++;
        sleep(1);
    }
    if(flag_d)
        fprintf(stderr, "Processo %s na CPU %d terminado\n", processo->nome, sched_getcpu());
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
    while (filho > 1 && processos[pai]->dt > aux->dt) {
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
        pthread_mutex_lock(&mutex_v[processo->id]);
        if(flag_d)
            fprintf(stderr, "Processo %s usando CPU: %d\n", processo->nome, sched_getcpu());
        processo->dt--;
        tempo++;
        sleep(1);
        if(processo->dt == 0 && flag_d)
            fprintf(stderr, "Processo %s na CPU %d terminado\n", processo->nome, sched_getcpu());
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

typedef struct node {
    Processo* processo;
    int contador;
    struct node* prox;
} Node;

int lista_vazia(Node* lista_first) {
    if(lista_first == NULL || lista_first->processo == NULL)
        return 1;
    return 0;
}

void lista_insere(Node** lista_first, Node** lista_last, Node* no) {
    if (lista_vazia((*lista_first))) {
        (*lista_first) = no;
        (*lista_first)->prox = (*lista_first);
        (*lista_last) = (*lista_first);
        return;
    }
    else if (!lista_vazia((*lista_first)) && (*lista_first) == (*lista_last)) {
        (*lista_last) = no;
        (*lista_last)->prox = (*lista_first);
        (*lista_first)->prox = (*lista_last);
        return;
    }
    (*lista_last)->prox = no;
    no->prox = (*lista_first);
    (*lista_last) = no;
}

void lista_remove(Node** lista_first, Node** lista_last) {
    if ((*lista_first) == (*lista_last)) {
        (*lista_first)->processo = NULL;
        return;
    }
    (*lista_last)->prox = (*lista_first)->prox;
    free((*lista_first)->processo);
    free((*lista_first));
    (*lista_first) = (*lista_last)->prox;
}


void* thread3(void * arg) {
    Node* no = (Node*)arg;
    while(no->processo->dt > 0) {
        //printf("Entrei na thread!\n");
        pthread_mutex_lock(&mutex_v[no->processo->id]);
        if(flag_d)
            fprintf(stderr, "Processo %s usando CPU: %d\n", no->processo->nome, sched_getcpu());
        no->processo->dt--;
        tempo++;
        no->contador += 1;
        sleep(1);
        if(no->processo->dt == 0 && flag_d)
            fprintf(stderr, "Processo %s na CPU %d terminado\n", no->processo->nome, sched_getcpu());
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main (int argc, char* argv[]) {
    if(argc == 5)
        flag_d = 1;

    FILE* ptr = fopen(argv[2], "r+");

    FILE* ptr2 = fopen(argv[3], "w+");

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
            if(flag_d)
                fprintf(stderr, "Novo processo: %s %d %d %d\n", processo1->nome, processo1->t0, processo1->dt, processo1->deadline);
            if(pthread_create(&t1, NULL, thread, processo1))
                printf("Dei erro 1!\n");
        }
        
        while(!queueEmpty()) {
            
            pthread_join(t1, NULL);
            if(flag_escreve) {
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
                fprintf(ptr2, "%s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
                flag_escreve = 0;
            }
            if(!queueEmpty() && processo1->dt == 0 && queueTop()->t0 <= tempo) {
                processo1 = queueRemove();
                flag_escreve = 1;
                if(flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo1->nome, processo1->t0, processo1->dt, processo1->deadline);
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
        
        if(flag_d)
            fprintf(stderr, "Mudancas de contexto: %lld\n", tam);
        fprintf(ptr2, "%lld\n", tam);

    } 
    else if (atoi(argv[1]) == 2) {
        long long int contador_contexto = 0;
        queueInit();
        tempo = 0;
        Processo* processo;
        long int contador = 0;
        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            
            if(strcmp(processo->nome, "")) {
                processo->id = contador;
                contador++;
                queueInsert(processo);
            }
            
        }

        t_v = malloc(queueSize()*sizeof(pthread_t));
        pthread_mutex_init(&mutex, NULL);
        mutex_v = malloc(queueSize()*sizeof(pthread_mutex_t));
        for(long long int i = 0; i < queueSize(); i++) {
            pthread_mutex_init(&mutex_v[i], NULL);
            pthread_mutex_lock(&mutex_v[i]);
            pthread_create(&t_v[i], NULL, thread2, queueItera(i));
        }
        
        Processo** heap_minimo;
        heap_minimo = malloc((queueSize()+1)*sizeof(Processo*));

        long int indice = 1;
        while(heap_minimo != NULL) {

            if(heap_empty(indice))
                pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex);
            processo = queueTop();
            Processo* aux = heap_minimo[1];

            if(heap_empty(indice)) {
                tempo++;
                sleep(1);
            }

            if(!heap_empty(indice) && aux->dt == 0) {
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                fprintf(ptr2, "%s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                heap_remove(heap_minimo, indice);
                indice--;
            }
            
            while(processo != NULL && processo->t0 <= tempo) {
                if (flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
                queueRemove();
                heap_insert(heap_minimo, indice, processo);
                indice++; 
                processo = queueTop();
            }

            if(!heap_empty(indice) && aux != heap_minimo[1])
                contador_contexto++;

            if (queueEmpty() && heap_empty(indice)) {
                free(heap_minimo);
                heap_minimo = NULL;
            }

            if(!heap_empty(indice))
                pthread_mutex_unlock(&mutex_v[heap_minimo[1]->id]);

            printf("\n");
        }
        for (int i = 0; i < queueSize(); i++) {
            pthread_join(t_v[i], NULL);
        }

        if(flag_d)
            fprintf(stderr, "Mundancas de contexto: %lld\n", contador_contexto);

        fprintf(ptr2, "%lld\n", contador_contexto);

    } 
    else {
        long long int contador_contexto = 0;
        queueInit();
        tempo = 0;
        Processo* processo;
        long int contador = 0;

        while(!feof(ptr)) {
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            
            if(strcmp(processo->nome, "")) {
                processo->id = contador;
                contador++;
                queueInsert(processo);
            }
            
        }

        Node** nos = malloc(queueSize()*sizeof(Node*));
        for(int i = 0; i < queueSize(); i++) {
            nos[i] = malloc(sizeof(Node));
        }

        t_v = malloc(queueSize()*sizeof(pthread_t));
        pthread_mutex_init(&mutex, NULL);
        mutex_v = malloc(queueSize()*sizeof(pthread_mutex_t));
        for(long long int i = 0; i < queueSize(); i++) {
            pthread_mutex_init(&mutex_v[i], NULL);
            pthread_mutex_lock(&mutex_v[i]);
            nos[i]->processo = queueItera(i);
            nos[i]->contador = 0;
            nos[i]->prox = NULL;
            pthread_create(&t_v[i], NULL, thread3, nos[i]);
        }
        
        Node* lista_first = malloc(sizeof(Node));
        lista_first->processo = NULL;
        lista_first->prox = NULL;

        Node* lista_last = lista_first;
        
        while(lista_first != NULL) {
            
            if(lista_vazia(lista_first))
                pthread_mutex_unlock(&mutex);
            
            pthread_mutex_lock(&mutex);
            processo = queueTop();
            Processo* auxiliar = lista_first->processo;
            
            if(lista_vazia(lista_first)) {
                tempo++;
                sleep(1);
            }

            if(!lista_vazia(lista_first) && lista_first->processo->dt == 0) {
                Processo* aux = lista_first->processo;
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                fprintf(ptr2, "%s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                lista_remove(&lista_first, &lista_last);
            }

            if(!lista_vazia(lista_first) && lista_first->contador >= quantum) {
                lista_first->contador = 0;
                if(lista_first != lista_last) {
                    lista_first = lista_first->prox;
                    lista_last = lista_last->prox;
                }
            }

            while(processo != NULL && processo->t0 <= tempo) {
                if (flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
                queueRemove();
                lista_insere(&lista_first, &lista_last, nos[processo->id]);
                processo = queueTop();
            }

            if(queueEmpty() && lista_vazia(lista_first)) {
                free(lista_first);
                lista_first = NULL;
            }

            if(!lista_vazia(lista_first)) {
                if(auxiliar != lista_first->processo)
                    contador_contexto++;
                pthread_mutex_unlock(&mutex_v[lista_first->processo->id]);
            }
        }

        for (int i = 0; i < queueSize(); i++) {
            pthread_join(t_v[i], NULL);
        }

        if(flag_d)
            fprintf(stderr, "Mundancas de contexto: %lld\n", contador_contexto);

        fprintf(ptr2, "%lld\n", contador_contexto);

    }

    fclose(ptr);
    fclose(ptr2);
    
    return 0;
}