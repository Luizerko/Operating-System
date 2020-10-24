#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

typedef struct coordenada {
    long int x;
    long int y;
} Coordenada;

typedef struct ciclista {
    long int identificador;
    /*
    posição muda quando um ciclista é removido da corrida, trocando a posição do último corredor do vetor para a posiição do primeiro.
    Isso acontece para que quando se remove a raiz ou algum nó interno a árvore continue formada e funcionando.
    */
    long int posicao_arvore;
    long int tipoNo;
    long int volta;
    long int velocidade;
    Coordenada coordenada;
} Ciclista;

_Atomic long int** velodromo;
long int* velocidades;
pthread_t* thread_ciclistas;
Ciclista* ciclistas;

long int d, n;
long int tamanho;
int* arrive;
int continua;

long int noventa_por_hora = -1;
int flag_noventa;

int mod(long int num, long int d) {
    if (num < 0)
        num = d + num;
    return num%d; 
}

//Um ciclista está do meu lado esquerdo
int lado(Ciclista* biker, int aux_y) {
    if(velodromo[biker->coordenada.x][aux_y] != -1 && ((velodromo[biker->coordenada.x][aux_y] == velodromo[mod(biker->coordenada.x+1, d)][aux_y]) ||
        (velodromo[biker->coordenada.x][aux_y] != velodromo[mod(biker->coordenada.x+1, d)][aux_y] && velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1)) )
        return 1;
    return 0;
}

//Um ciclista está do meu lado esquerdo com a cabeça no meu cu
int lado_mais_um(Ciclista* biker, int aux_y) {
    if(velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1 && ((velodromo[mod(biker->coordenada.x+1, d)][aux_y] == velodromo[mod(biker->coordenada.x+2, d)][aux_y]) ||
        (velodromo[mod(biker->coordenada.x+1, d)][aux_y] != velodromo[mod(biker->coordenada.x+2, d)][aux_y] && velodromo[mod(biker->coordenada.x+2, d)][aux_y] != -1)) )
        return 1;
    return 0;
}

void instrucao(Ciclista* biker) {
    Coordenada coord_anterior;
    coord_anterior.x = biker->coordenada.x;
    coord_anterior.y = biker->coordenada.y;

    if(biker->volta < 2) {//Velocidade é 30km/h.
        if(biker->coordenada.x - 1 <= 0)
            biker->volta++;
        biker->coordenada.x = mod(biker->coordenada.x-1, d);
        printf("Eu, %ld, tenho coordenada: %ld\n", biker->identificador, biker->coordenada.x);

        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;

        if(velodromo[(coord_anterior.x+1)%d][coord_anterior.y] == biker->identificador)
            velodromo[(coord_anterior.x+1)%d][coord_anterior.y] = -1;

        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, d)][biker->coordenada.y] = biker->identificador;
        
        arrive[biker->posicao_arvore] = 1;

        printf("Eu, %ld, passei pela volta %ld\n", biker->identificador, biker->volta);
    }

    else if(biker->volta < n ) {
        
        if(biker->velocidade == 30) {

            biker->coordenada.x = mod(biker->coordenada.x-1, d);
            int aux_y = 0;
            
            while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                    (lado_mais_um(biker, aux_y)) ||
                    (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                    (velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1 && arrive[ciclistas[velodromo[mod(biker->coordenada.x+1, d)][aux_y]].posicao_arvore]))) {
                aux_y++;
            }
            
            biker->coordenada.y = aux_y;
        }
        else if(biker->velocidade == 60) {
            int aux_y = biker->coordenada.y;
            
            if((velodromo[mod(biker->coordenada.x-1, d)][aux_y] == -1 || (ciclistas[velodromo[mod(biker->coordenada.x-1, d)][aux_y]].velocidade == 60 && !arrive[ciclistas[velodromo[mod(biker->coordenada.x-1, d)][aux_y]].posicao_arvore])) && 
                (velodromo[mod(biker->coordenada.x-2, d)][aux_y] == -1 || !arrive[ciclistas[velodromo[mod(biker->coordenada.x-2, d)][aux_y]].posicao_arvore])) {

                biker->coordenada.x = mod(biker->coordenada.x-2, d);
                
                aux_y = 0;
                while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                        (velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1 && arrive[ciclistas[velodromo[mod(biker->coordenada.x+1, d)][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                
                biker->coordenada.y = aux_y;
            }
            else {
                while(aux_y < 10 && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                        (velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1 && arrive[ciclistas[velodromo[mod(biker->coordenada.x+1, d)][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                if(aux_y == 10) {//Anda a 30 km/h nessa iteração.
                    biker->coordenada.x = (biker->coordenada.x - 1)%d;
                    
                    aux_y = 0;
                    while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                        (velodromo[mod(biker->coordenada.x+1, d)][aux_y] != -1 && arrive[ciclistas[velodromo[mod(biker->coordenada.x+1, d)][aux_y]].posicao_arvore]))) {
                        aux_y++;
                    }
                }

                biker->coordenada.y = aux_y;
            }
        }

        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;
        if(velodromo[(coord_anterior.x+1)%d][coord_anterior.y] == biker->identificador)
            velodromo[(coord_anterior.x+1)%d][coord_anterior.y] = -1;
        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, d)][biker->coordenada.y] = biker->identificador;
        
        if(coord_anterior.x < biker->coordenada.x) {
            biker->volta++;
            if(biker->velocidade == 30) {
                if(rand()%10 < 8)
                    biker->velocidade = 60;
            }
            else {
                if(rand()%10 < 4)
                    biker->velocidade = 30;
            }
        }

        arrive[biker->posicao_arvore] = 1;
    }

    /*else { //Duas últimas voltas
        if(flag_noventa != 1) {
            if(rand()%10 == 0)
                noventa_por_hora = rand()%(tamanho);                
            flag_noventa = 1;
        }
        
        if(biker->posicao_arvore == noventa_por_hora)
            biker->velocidade = 90;
            
        if(biker->velocidade == 30) {
            if(biker->coordenada.x - 1 <= 0) {
                biker->volta++;
                if(rand()%10 < 8)
                    biker->velocidade = 60;
            }
            biker->coordenada.x = (biker->coordenada.x - 1)%d;
        }
        else if(biker->velocidade == 60) {
            if(biker->coordenada.x - 2 <= 0) {
                biker->volta++;
                if(rand()%10 < 4)
                    biker->velocidade = 30;
            }
            biker->coordenada.x = (biker->coordenada.x - 2)%d;
        }
        else {
            if(biker->coordenada.x - 3 <= 0)
                biker->volta++;
            biker->coordenada.x = (biker->coordenada.x - 3)%d;
        }

    }*/

    arrive[biker->posicao_arvore] = 1;
}

void* thread_ciclista(void* arg) {
    int round = 0;//interaçao e continua globais.
    Ciclista* biker = (Ciclista*)arg;

    while(1) {

        if(biker->posicao_arvore == 1)
            biker->tipoNo = 1;//Raiz.
        else if(2*biker->posicao_arvore < tamanho)
            biker->tipoNo = 2;//Interno.
        else
            biker->tipoNo = 3;//Folha

        if (biker->tipoNo == 3) { //Folha

            //printf("ENTREI FOLHA! %ld\n", biker->identificador);

            instrucao(biker);
            //arrive[biker->posicao_arvore] = 1;
            
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

        else if (biker->tipoNo == 2) { //Interno

            //printf("ENTREI INTERNO! %ld\n", biker->identificador);
            
            if(2*biker->posicao_arvore < tamanho && arrive[2*biker->posicao_arvore] != -1) {
                while(arrive[2*biker->posicao_arvore] == 0) {
                    usleep(d*40*(1000-1));
                }
                arrive[2*biker->posicao_arvore] = 0;
            }

            if(2*biker->posicao_arvore + 1 < tamanho && arrive[2*biker->posicao_arvore + 1] != -1) {
                while(arrive[2*biker->posicao_arvore + 1] == 0) {
                    usleep(d*40*(1000-1));
                }
                arrive[2*biker->posicao_arvore + 1] = 0;
            }

            instrucao(biker);
            //arrive[biker->posicao_arvore] = 1;

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

            //printf("ENTREI RAIZ! %ld\n", biker->identificador);

            if(2*biker->posicao_arvore < tamanho && arrive[2*biker->posicao_arvore] != -1) {
                while(arrive[2*biker->posicao_arvore] == 0) {
                    usleep(d*40*(1000-1));
                }
                arrive[2*biker->posicao_arvore] = 0;
            }

            if(2*biker->posicao_arvore + 1 < tamanho && arrive[2*biker->posicao_arvore + 1] != -1) {
                while(arrive[2*biker->posicao_arvore + 1] == 0) {
                    usleep(d*40*(1000-1));
                }
                arrive[2*biker->posicao_arvore + 1] = 0;
            }

            instrucao(biker);
            //arrive[biker->posicao_arvore] = 1;
            
            printf("\n----------INICIEI VELÓDROMO----------\n");
            for(int i = 0; i < d; i++) {
                for(int j = 0; j < 10; j++) {
                    printf("%ld ", velodromo[i][j]);
                }
                printf("\n");
            }
            printf("\n----------FINALIZEI VELÓDROMO----------\n");

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

    d = (long int)atoi(argv[1]);
    n = (long int)atoi(argv[2]);
    tamanho = n;

    ciclistas = malloc((n+1)*sizeof(Ciclista));
    thread_ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(2*d*sizeof(long int*));
    for(long int i = 0; i < d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }
    //velocidades = malloc(n*sizeof(long int));
    for(long int i = 1; i <= n; i++) {
        ciclistas[i].velocidade = 30;
        ciclistas[i].volta = 0;
        ciclistas[i].identificador = i;
    }
    continua = 0;
    arrive = malloc((n+1)*sizeof(int));
    for(long int i = 1; i <= n; i++) {
        arrive[i] = 0;
    }

    long int tam = n;
    long int* sorteados = malloc((n+1)*sizeof(long int));
    for(long int i = 1; i <= n; i++) {
        sorteados[i] = i;
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
            long int aux_2 = (rand()%tam)+1;
            ciclistas[sorteados[aux_2]].posicao_arvore = sorteados[aux_2];
            ciclistas[sorteados[aux_2]].coordenada.x = i;
            ciclistas[sorteados[aux_2]].coordenada.y = j;
            velodromo[i][j] = sorteados[aux_2];
            velodromo[i+1][j] = sorteados[aux_2];
            sorteados[aux_2] = sorteados[tam];
            tam--;
        }
    }

    /* Printa velódromo
    printf("\n----------INICIEI VELÓDROMO----------\n");
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < 10; j++) {
            printf("%ld ", velodromo[i][j]);
        }
        printf("\n");
    }
    printf("\n----------FINALIZEI VELÓDROMO----------\n");
    */

    for(long int i = 0; i < n; i++) {
        pthread_create(&thread_ciclistas[i], NULL, thread_ciclista, &ciclistas[i+1]);
    }
    
    for(long int i = 0; i < n; i++) {
        pthread_join(thread_ciclistas[i], NULL);
    }

    free(sorteados);

    return 0;
}