#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

long int** velodromo;
pthread_t* ciclistas;

int* iteracao;
int continua;

int volta = 1;

typedef struct coordenada {
    long int x;
    long int y;
} Coordenada;

void* ciclista(void* arg) {
    int round = 0;//interaçao e continua globais.
    long int *dados = (long int*)arg;
    long int identificador = dados[0];
    long int posicao = identificador;
    long int d = dados[1]; 
    long int tamanho = dados[2];
    long int tipoNo;
    long int velocirapdo = 30;
    Coordenada coordenada;
    coordenada.x = dados[3];
    coordenada.y = dados[4];

    while(1) {

        if(posicao == 0)
            tipoNo = 0;//Raiz.
        else if(2*posicao < tamanho)
            tipoNo = 1;//Interno.
        else
            tipoNo = 2;//Folha

        if (tipoNo == 2) { //Folha
            iteracao[posicao] = 1;

            //instrução
            if(velocirapdo == 30) {
                coordenada.x-=1;
            }
            else if(velocirapdo == 60) {
                coordenada.x-=2;
            }
            else {
                coordenada.x-=3;
            }
            
            if (round == 0) {
                while (continua == 0) {
                    usleep(d*40*(1000-1));//??
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*40*(1000-1));//??
                }
                round = 0;                
            }
        }

        else if (tipoNo == 1) { //Interno
            if(2*posicao < tamanho && iteracao[2*posicao] != -1) {
                while(iteracao[2*posicao] == 0) {
                    usleep(d*40*(1000-1));
                }
                iteracao[2*posicao] = 0;
            }

            if(2*posicao + 1 < tamanho && iteracao[2*posicao + 1] != -1) {
                while(iteracao[2*posicao + 1] == 0) {
                    usleep(d*40*(1000-1));
                }
                iteracao[2*posicao + 1] = 0;
            }

            iteracao[posicao] = 1;

            //instrução.
            
            if (round == 0) {
                while (continua == 0) {
                    usleep(d*40*(1000-1));//??
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*40*(1000-1));//??
                }
                round = 0;                
            }
        }

        else { //Raiz
            if(2*posicao < tamanho && iteracao[2*posicao] != -1) {
                while(iteracao[2*posicao] == 0) {
                    usleep(d*40*(1000-1));
                }
                iteracao[2*posicao] = 0;
            }

            if(2*posicao + 1 < tamanho && iteracao[2*posicao + 1] != -1) {
                while(iteracao[2*posicao + 1] == 0) {
                    usleep(d*40*(1000-1));
                }
                iteracao[2*posicao + 1] = 0;
            }

            //instrucao.
            
            if(round == 0) {
                continua = 1;
                round = 1;
            }
            else {
                continua = 0;
                round = 0;
            }
        }
    }
}

int main (int argc, char* argv[]) {

    long int d, n;
    d = (long int)atoi(argv[1]);
    n = (long int)atoi(argv[2]);

    ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(2*d*sizeof(long int*));
    for(long int i = 0; i < d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }

    continua = 0;
    iteracao = malloc(n*sizeof(int));
    for(long int i = 0; i < n; i++) {
        iteracao[i] = 0;
    }

    long int tam = n;
    long int* sorteados = malloc(n*sizeof(long int));
    for(long int i = 0; i < n; i++) {
        sorteados[i] = i;
    }

    //Dados de cada um dos ciclistas para serem passados como parametro para o pthread_create.
    long int** dados = malloc(n*sizeof(long int*));
    for(long int i = 0; i < n; i++) {
        dados[i] = malloc(5*sizeof(long int));
    }

    //Variável que conta o número de linhas a serem inicializadas baseado no número de ciclistas. O número 
    //de linhas onde haverão ciclistas inicialmente é 2 vezes o teto do número de ciclistas dividido por 5,
    //já que podemos colocar 5 ciclistas a cada 2 linhas no máximo.
    //(cada ciclista ocupa 2 linhas, sendo cada linha um espaço de comprimento 0,5 metros).
    long int aux;
    if(n%5 == 0)
        aux = 2*(n/5);
    else
        aux = 2*((n/5) + 1);

    for(long int i = 0; i < aux && tam > 0; i+=2) {
        for(long int j = 0; j < 5 && tam > 0; j++) {
            long int aux_2 = rand()%tam;
            dados[sorteados[aux_2]][0] = sorteados[aux_2];
            dados[sorteados[aux_2]][1] = d;
            dados[sorteados[aux_2]][2] = n;
            dados[sorteados[aux_2]][3] = i + 1;
            dados[sorteados[aux_2]][4] = j;
            velodromo[i][j] = sorteados[aux_2];
            velodromo[i+1][j] = sorteados[aux_2];
            sorteados[aux_2] = sorteados[tam-1];
            tam--;
        }
    }

    for(long int i = 0; i < n; i++) {
        //pthread_create(&ciclistas[i], NULL, ciclista, &dados[i]);
        
    }

    /* Printa velódromo
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < 10; j++) {
            printf("%ld ", velodromo[i][j]);
        }
        printf("\n");
    }
    */

    free(sorteados);

    return 0;
}



//Se pa a gente tem que ir pensando na velocidade nova do ciclista em ordem dos que tao na frente pros que taoa tras. se o da frente ta com 30, o de tras já é 30 se ele n puder ultrapassar.
//