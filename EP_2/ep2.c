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

//Um ciclista está do meu lado esquerdo
int lado(Ciclista* biker, int aux_y) {
    if(velodromo[biker->coordenada.x][aux_y] != -1 && ((velodromo[biker->coordenada.x][aux_y] == velodromo[(biker->coordenada.x+1)%d][aux_y]) ||
        (velodromo[biker->coordenada.x][aux_y] != velodromo[(biker->coordenada.x+1)%d][aux_y] && velodromo[(biker->coordenada.x+1)%d][aux_y] != -1)) )
        return 1;
    return 0;
}

//Um ciclista está do meu lado esquerdo com a cabeça no meu cu
int lado_mais_um(Ciclista* biker, int aux_y) {
    if(velodromo[(biker->coordenada.x+1)%d][aux_y] != -1 && ((velodromo[(biker->coordenada.x+1)%d][aux_y] == velodromo[(biker->coordenada.x+2)%d][aux_y]) ||
        (velodromo[(biker->coordenada.x+1)%d][aux_y] != velodromo[(biker->coordenada.x+2)%d][aux_y] && velodromo[(biker->coordenada.x+2)%d][aux_y] != -1)) )
        return 1;
    return 0;
}

void* thread_ciclista(void* arg) {
    int round = 0;//interaçao e continua globais.
    Ciclista* biker = (Ciclista*)arg;

    /*long int *dados = (long int*)arg;
    long int identificador = dados[0];
    long int posicao = identificador; //posição muda quando um ciclista é removido da corrida, trocando a posição do último corredor do vetor para a posiição do primeiro.
    //Isso acontece para que quando se remove a raiz ou algum nó interno a árvore continue formada e funcionando.
    long int d = dados[1]; 
    long int tamanho = dados[2];
    long int tipoNo;
    long int volta = 0;
    Coordenada coordenada;
    coordenada.x = dados[3];
    coordenada.y = dados[4];*/

    while(1) {

        int flag_volta = 0;

        if(biker->posicao_arvore == 0)
            biker->tipoNo = 0;//Raiz.
        else if(2*biker->posicao_arvore < tamanho)
            biker->tipoNo = 1;//Interno.
        else
            biker->tipoNo = 2;//Folha

        if (biker->tipoNo == 2) { //Folha

            Coordenada coord_anterior;
            coord_anterior.x = biker->coordenada.x;
            coord_anterior.y = biker->coordenada.y;

            if(biker->volta < 2) {//Velocidade é 30km/h.
                if(biker->coordenada.x - 1 <= 0)
                    biker->volta++;
                biker->coordenada.x = (biker->coordenada.x - 1)%d;
                velodromo[coord_anterior.x][coord_anterior.y] = -1;
                if(velodromo[(coord_anterior.x+1)%d][coord_anterior.y] == biker->identificador)
                    velodromo[(coord_anterior.x+1)%d][coord_anterior.y] = -1;
                velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
                velodromo[(biker->coordenada.x+1)%d][biker->coordenada.y] = biker->identificador;
                
                arrive[biker->posicao_arvore] = 1;
            }

            else if(biker->volta < n - 2) {
                
                if(biker->velocidade == 30) {

                    biker->coordenada.x = (biker->coordenada.x - 1)%d;
                    int aux_y = 0;
                    
                    while(aux_y < biker->coordenada.y && ((lado_esq(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                          (lado_mais_um_esq(biker, aux_y)) ||
                          (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                          (velodromo[biker->coordenada.x+1][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x+1][aux_y]].posicao_arvore]))) {
                        aux_y++;
                    }
                    
                    biker->coordenada.y = aux_y;
                }
                else if(biker->velocidade == 60) {
                    int aux_y = biker->coordenada.y;
                    
                    if((velodromo[(biker->coordenada.x-1)%d][aux_y] == -1 || (ciclistas[velodromo[(biker->coordenada.x-1)%d][aux_y]].velocidade == 60 && !arrive[ciclistas[velodromo[(biker->coordenada.x-1)%d][aux_y]].posicao_arvore])) && 
                       (velodromo[(biker->coordenada.x-2)%d][aux_y] == -1 || !arrive[ciclistas[velodromo[(biker->coordenada.x-2)%d][aux_y]].posicao_arvore])) {

                        biker->coordenada.x = (biker->coordenada.x - 2)%d;
                        
                        aux_y = 0;
                        while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                             (lado_mais_um(biker, aux_y)) ||
                             (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                             (velodromo[biker->coordenada.x+1][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x+1][aux_y]].posicao_arvore]))) {
                            aux_y++;
                        }
                        
                        biker->coordenada.y = aux_y;
                    }
                    else {
                        while(aux_y < 10 && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                             (lado_mais_um(biker, aux_y)) ||
                             (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                             (velodromo[biker->coordenada.x+1][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x+1][aux_y]].posicao_arvore]))) {
                            aux_y++;
                        }
                        if(aux_y == 10) {//Anda a 30 km/h nessa iteração.
                            biker->coordenada.x = (biker->coordenada.x - 1)%d;
                            
                            aux_y = 0;
                            while(aux_y < biker->coordenada.y && ((lado_esq(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                                (lado_mais_um_esq(biker, aux_y)) ||
                                (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]) ||
                                (velodromo[biker->coordenada.x+1][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x+1][aux_y]].posicao_arvore]))) {
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
                velodromo[(biker->coordenada.x+1)%d][biker->coordenada.y] = biker->identificador;
                
                if(coord_anterior.x < biker->coordenada.x) {
                    biker->volta++;
                    if(biker->velocidade == 30)
                        if(rand()%10 < 8)
                            biker->velocidade = 60;
                    else
                        if(rand()%10 < 4)
                            biker->velocidade = 30;
                }

                arrive[biker->posicao_arvore] = 1;
            }

            else { //Duas últimas voltas
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

            }

            arrive[biker->posicao_arvore] = 1;
            
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

        else if (biker->tipoNo == 1) { //Interno
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

            arrive[biker->posicao_arvore] = 1;



            //instrução.
            if(biker->velocidade == 30) {
                biker->coordenada.x-=1;
            }
            else if(biker->velocidade == 60) {
                biker->coordenada.x-=2;
            }
            else {
                biker->coordenada.x-=3;
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

        else { //Raiz
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

            //instrucao.
            if(biker->velocidade == 30) {
                biker->coordenada.x-=1;
            }
            else if(biker->velocidade == 60) {
                biker->coordenada.x-=2;
            }
            else {
                biker->coordenada.x-=3;
            }
            
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

    ciclistas = malloc(n*sizeof(Ciclista));
    thread_ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(2*d*sizeof(long int*));
    for(long int i = 0; i < d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }
    //velocidades = malloc(n*sizeof(long int));
    for(long int i = 0; i < n; i++) {
        ciclistas[i].velocidade = 30;
        ciclistas[i].volta = 0;
        ciclistas[i].identificador = i;
    }
    continua = 0;
    arrive = malloc(n*sizeof(int));
    for(long int i = 0; i < n; i++) {
        arrive[i] = 0;
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
            ciclistas[sorteados[aux_2]].posicao_arvore = sorteados[aux_2];
            ciclistas[sorteados[aux_2]].coordenada.x = i + 1;
            ciclistas[sorteados[aux_2]].coordenada.y = j;
            velodromo[i][j] = sorteados[aux_2];
            velodromo[i+1][j] = sorteados[aux_2];
            sorteados[aux_2] = sorteados[tam-1];
            tam--;
        }
    }

    for(long int i = 0; i < n; i++) {
        //pthread_create(&thread_ciclistas[i], NULL, thread_ciclista, &ciclistas[i]);
        
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