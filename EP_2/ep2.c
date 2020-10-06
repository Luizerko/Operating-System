#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

long int** velodromo;
pthread_t* ciclistas;

int* iteracao;
int* continua;

/*typedef struct ciclista{
    int identificador;
    int x;
    int y;
    int velocidade;
} Ciclista;*/

void* coordena(void* arg) {
    long int *entradas = (long int*)arg;
    long int n = entradas[0];
    long int d = entradas[1];
    while(1) {
        long int soma = 0;
        for(long int i = 0; i < n; i++) {
            soma += iteracao[i];
        }
        if (soma == n)
            for (long int i = 0; i < n; i++) {
                iteracao[i] = 0;
                continua[i] = 1;
            }
        else
            usleep(d*40*(1000-1));
    }
}

void* ciclista(void* arg) {
    long int *dados = (long int*)arg;
    long int identificador = dados[0];
    long int d = dados[1]; 
    int velocidade = 30, x, y;
    iteracao[identificador] = 1;
    while(!continua[identificador]) {
        usleep(d*40*(1000-1));
    }
}

int main (int argc, char* argv[]) {

    long int d, n;
    d = (long int)atoi(argv[1]);
    n = (long int)atoi(argv[2]);

    ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(d*sizeof(long int*));
    for(long int i = 0; i < d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }

    iteracao = malloc(n*sizeof(int));
    continua = malloc(n*sizeof(int));
    for(long int i = 0; i < n; i++) {
        iteracao[i] = 0;
        continua[i] = 0;
    }

    pthread_t coordenador;
    long int entradas[2] = {n, d};
    pthread_create(&coordenador, NULL, coordena, &entradas);

    for(long int i = 0; i < n; i++) {
        long int dados[2] = {i, d};
        pthread_create(&ciclistas[i], NULL, ciclista, &dados);
        
    }

    long int tam = n;
    long int* sorteados = malloc(n*sizeof(long int));
    for(long int i = 0; i < n; i++) {
        sorteados[i] = i;
    }

    long int aux;
    if(n%5 == 0)
        aux = n/5;
    else
        aux = (n/5) + 1;
    

    for(long int i = 0; i < aux; i++) {
        for(int j = 0; j < 5; j++) {
            int aux_2 = rand()%tam;
            velodromo[i][j] = sorteados[aux_2];
            sorteados[aux_2] = sorteados[tam-1];
            tam--;
        }
    }
    free(sorteados);

    return 0;
}



//Se pa a gente tem que ir pensando na velocidade nova do ciclista em ordem dos que tao na frente pros que taoa tras. se o da frente ta com 30, o de tras já é 30 se ele n puder ultrapassar.
//