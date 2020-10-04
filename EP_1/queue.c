#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "processo.h"

static Processo** vetor;
static long long int tam;
static long long int ini;
static long long int fim;
static long long int n;


//Realoca o vetor que comporta a fila para um vetor de tamanho igual a novo_tamanho.
static void resize(long long int novo_tamanho) {
    Processo** auxiliar = malloc(novo_tamanho*sizeof(Processo*));
    long long int iterador = 0;
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
        //Caso o número de elementos da fila ultrapasse o tamanho do vetor onde a fila é implementada, usa a função
        //resize para realocar o vetor como um vetor com tamanho dobrado em relação ao anterior.
        if(!queueEmpty() && fim == ini)
            resize(2*tam);  
        vetor[fim] = processo;
        fim = (fim+1)%tam;
        n++;
    }
}

//Remove o processo inicial da fila caso ela não esteja vazia, retornando-o. Caso a fila esteja vazia, retorna NULL. 
//Caso o novo número de processos seja menor que o tamanho do vetor da fila dividido por 4, realoca o vetor para um 
//de tamanho 2 vezes menor. 
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

Processo* queueTop() {
    if(!queueEmpty()) {
        return vetor[ini];
    }
    return NULL;
}

//Retorna o Processo de posição indice da fila caso essa posição existe e NULL caso a posição não exista. 
Processo* queueItera(long int indice) {
    if(!queueEmpty() && indice < queueSize()) {
        return vetor[indice];
    }
    return NULL;
}

bool queueEmpty() {
    if(n > 0)
        return 0;
    return 1;
}

long long int queueSize() {
    return n;
}

void queueFree() {
    free(vetor);
}