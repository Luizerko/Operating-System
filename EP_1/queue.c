#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "processo.h"

static Processo** vetor;
static int tam;
static int ini;
static int fim;
static int n;

static void resize(int novo_tamanho) {
    Processo** auxiliar = malloc(novo_tamanho*sizeof(Processo*));
    int iterador = 0;
    if(n == tam) {
        while(iterador < tam) {
            auxiliar[iterador] = vetor[ini];
            iterador++;
            ini = (ini + 1)%tam;
        }
        ini = 0;
        fim = n;
        tam = novo_tamanho;
        free(vetor);
        vetor = auxiliar;
    }
    else {
        while(ini != fim) {
            auxiliar[iterador] = vetor[ini];
            iterador++;
            ini = (ini + 1)%tam;
        }
        ini = 0;
        fim = n;
        tam = novo_tamanho;
        free(vetor);
        vetor = auxiliar;
    }
}

void queueInit() {
    vetor = NULL;
    tam = 0;
    ini = 0;
    fim = 0;
    n = 0;
}

void queueInsert(Processo* processo) {
    if(tam == 0) {
        vetor = malloc(sizeof(Processo*));
        tam = 1;
        n = 1;
        vetor[fim] = processo;
        fim = (fim+1)%tam;
    }
    else {
        if(!queueEmpty() && fim == ini)
            resize(2*tam);
        vetor[fim] = processo;
        fim = (fim+1)%tam;
        n++;
    }
}

Processo* queueRemove() {
    if(!queueEmpty()) {
        if(n <= tam/4)
            resize(tam/2);
        n--;
        Processo* auxiliar = vetor[ini];
        ini = (ini+1)%tam;
        return auxiliar;
    }
    return NULL;
}

bool queueEmpty() {
    if(n > 0)
        return 0;
    return 1;
}

int queueSize() {
    return n;
}

void queueFree() {
    free(vetor);
}