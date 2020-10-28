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

typedef struct final_ciclista {
    long int identificador;
    long int rank;
    long int tempo_eliminacao;
    long int volta_quebra;
} Final_Ciclista;

_Atomic long int** velodromo;
pthread_mutex_t mutex;
long int* velocidades;
pthread_t* thread_ciclistas;
Ciclista* ciclistas;
int* arrive;

long int d, n;
long int tempo = 0;
int saida_completa = 0;
long int* voltas_eliminacao;
Final_Ciclista* vetor_final;
long int aux_ini, aux_fim;
long int tamanho;
int continua;
long int ultimo_arvore;

long int noventa_por_hora = -1;
int flag_noventa;

int mod(long int num, long int d) {
    if (num < 0)
        num = d + num;
    return num%d; 
}

//Um ciclista está do meu lado esquerdo
int lado(Ciclista* biker, int aux_y) {
    if(velodromo[biker->coordenada.x][aux_y] != -1 && ((velodromo[biker->coordenada.x][aux_y] == velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y]) ||
        (velodromo[biker->coordenada.x][aux_y] != velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] && velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != -1)) )
        return 1;
    return 0;
}

//Um ciclista está do meu lado esquerdo com a cabeça no meu cu
int lado_mais_um(Ciclista* biker, int aux_y) {
    if(velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != -1 && ((velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] == velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y]) ||
        (velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y] && velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y] != -1)) )
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
        biker->coordenada.x = mod(biker->coordenada.x-1, 2*d);

        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;

        if(velodromo[mod((coord_anterior.x+1), 2*d)][coord_anterior.y] == biker->identificador)
            velodromo[mod((coord_anterior.x+1), 2*d)][coord_anterior.y] = -1;

        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = biker->identificador;

    }

    else if(biker->volta < 2*n) {
        
        if(biker->velocidade == 30) {

            biker->coordenada.x = mod(biker->coordenada.x-1, 2*d);
            int aux_y = 0;
            
            while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                    (lado_mais_um(biker, aux_y)) ||
                    (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                aux_y++;
            }
            
            biker->coordenada.y = aux_y;
        }
        else if(biker->velocidade == 60) {
            int aux_y = biker->coordenada.y;
            
            if((velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y] == -1 || (ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].velocidade == 60 && !arrive[ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].posicao_arvore])) && 
                (velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y] == -1 || !arrive[ciclistas[velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y]].posicao_arvore])) {
                
                biker->coordenada.x = mod(biker->coordenada.x-2, 2*d);
                
                aux_y = 0;
                while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                biker->coordenada.y = aux_y;

            }
            else {
                while(aux_y < 10 && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                if(aux_y == 10) {//Anda a 30 km/h nessa iteração.
                    biker->coordenada.x = mod((biker->coordenada.x - 1), 2*d);
                    
                    aux_y = 0;
                    while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                        aux_y++;
                    }
                }

                else {
                    aux_y++;
                    biker->coordenada.x = mod(biker->coordenada.x-2, 2*d);
                }

                biker->coordenada.y = aux_y;

            }
        }

        printf("chegueeeei %ld\n", biker->identificador);

        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;
        if(velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] == biker->identificador)
            velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] = -1;
        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = biker->identificador;
        
        if(coord_anterior.x < biker->coordenada.x) {
            biker->volta++;
            printf("volta: %ld\n", biker->volta);
            if(biker->volta%2 != 0) {
                if(voltas_eliminacao[biker->volta] == tamanho - 1) {//Remoção deste ciclista da corrida.
                    tamanho--;
                    vetor_final[aux_fim].identificador = biker->identificador;
                    //isso aqui q eu mudei, tava biker->identificador e troquei pra aux_fim
                    vetor_final[aux_fim].tempo_eliminacao = tempo+20; //Elimina com uma passagem de tempo extra, já que considera a passagem de tempo da iteração atual em que atingiu a nova volta e foi eliminado.
                    aux_fim--;
                    
                    velodromo[biker->coordenada.x][biker->coordenada.y] = -1;
                    velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = -1;
                    
                    printf("meu id: %ld\n", biker->identificador);
                    for(int j = 1; j <= tamanho+1; j++) {
                        printf("id:%ld arrive:%d\n", ciclistas[j].identificador, arrive[j]);
                    }

                    int flag_raiz = 0;
                    if(biker->posicao_arvore == 1)//É a raiz.
                        flag_raiz = 1; 

                    arrive[biker->posicao_arvore] = arrive[ciclistas[ultimo_arvore].posicao_arvore];
                    ciclistas[ultimo_arvore].posicao_arvore = biker->posicao_arvore;
                    for(long int w = 1; w <= tamanho+1; w++) {
                        if(ciclistas[w].posicao_arvore == tamanho) {
                            ultimo_arvore = w;
                            break;
                        }
                    }

                    if(flag_raiz) {
                        for(int j = 1; j <= tamanho+1; j++)
                            arrive[j] = 0;
                        continua = 1 - continua;
                    }   
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }
                else
                    voltas_eliminacao[biker->volta]++;
            }
            if(biker->velocidade == 30) {
                if(rand()%10 < 8)
                    biker->velocidade = 60;
            }
            else {
                if(rand()%10 < 4)
                    biker->velocidade = 30;
            }
        }
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

        if (tamanho == 1) {
            vetor_final[aux_ini].identificador = biker->identificador;
            pthread_exit(NULL);
        }
        if(biker->posicao_arvore == 1)
            biker->tipoNo = 1;//Raiz.
        else if(2*biker->posicao_arvore <= tamanho)
            biker->tipoNo = 2;//Interno.
        else
            biker->tipoNo = 3;//Folha

        if (biker->tipoNo == 3) { //Folha

            pthread_mutex_lock(&mutex);
            instrucao(biker);

            /*
            printf("\n----------INICIEI VELÓDROMO %ld----------\n", biker->identificador);
            for(int i = 0; i < d; i++) {
                for(int j = 0; j < 10; j++) {
                    printf("%ld ", velodromo[i][j]);
                }
                printf("\n");
            }
            printf("\n----------FINALIZEI VELÓDROMO %ld----------\n", biker->identificador);
            */

            pthread_mutex_unlock(&mutex);
            
            if (round == 0) {
                while (continua == 0) {
                    usleep(d*10*(5-1));//??
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*10*(5-1));//??
                }
                round = 0;                
            }
        }

        else if (biker->tipoNo == 2) { //Interno
            
            if(2*biker->posicao_arvore <= tamanho) {
                while(arrive[2*biker->posicao_arvore] == 0) {
                    usleep(d*10*(5-1));
                }
            }

            if(2*biker->posicao_arvore + 1 <= tamanho) {
                while(arrive[2*biker->posicao_arvore + 1] == 0) {
                    usleep(d*10*(5-1));
                }
            }

            pthread_mutex_lock(&mutex);
            instrucao(biker);

            /*
            printf("\n----------INICIEI VELÓDROMO %ld----------\n", biker->identificador);
            for(int i = 0; i < d; i++) {
                for(int j = 0; j < 10; j++) {
                    printf("%ld ", velodromo[i][j]);
                }
                printf("\n");
            }
            printf("\n----------FINALIZEI VELÓDROMO %ld----------\n", biker->identificador);
            */

            pthread_mutex_unlock(&mutex);

            if (round == 0) {
                while (continua == 0) {
                    usleep(d*10*(5-1));//??
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*10*(5-1));//??
                }
                round = 0;                
            }
        }

        else { //Raiz

            if(2*biker->posicao_arvore <= tamanho) {
                while(arrive[2*biker->posicao_arvore] == 0) {
                    usleep(d*40*(5-1));
                }
                //arrive[2*biker->posicao_arvore] = 0;
            }

            if(2*biker->posicao_arvore + 1 <= tamanho) {
                while(arrive[2*biker->posicao_arvore + 1] == 0) {
                    usleep(d*40*(5-1));
                }
                //arrive[2*biker->posicao_arvore + 1] = 0;
            }

            pthread_mutex_lock(&mutex);
            instrucao(biker);
            tempo += 20;
            
            if(saida_completa) {
                printf("\n----------INICIEI VELÓDROMO %ld----------\n", biker->identificador);
                for(int i = 0; i < 2*d; i++) {
                    for(int j = 0; j < 10; j++) {
                        printf("%ld ", velodromo[i][j]);
                    }
                    printf("\n");
                }
                printf("\n----------FINALIZEI VELÓDROMO %ld----------\n", biker->identificador);
            }
              
            for (int i = 1; i <= n; i++) {
                arrive[i] = 0;
            }
            pthread_mutex_unlock(&mutex);
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
    if(argc > 3) {
        saida_completa = 1;
    }
    tamanho = n;
    ultimo_arvore = n;
    pthread_mutex_init(&mutex, NULL);
    srand(1);

    ciclistas = malloc((n+1)*sizeof(Ciclista));
    thread_ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(2*d*sizeof(long int*)); 
    for(long int i = 0; i < 2*d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }
    voltas_eliminacao = malloc(2*n*sizeof(long int));
    for(long int i = 0; i < 2*n; i++) {
        voltas_eliminacao[i] = 0;
    }
    aux_fim = n-1;
    aux_ini = 0;
    vetor_final = malloc(n*sizeof(Final_Ciclista));
    for (long int i = 0; i < n; i++) {
        vetor_final[i].volta_quebra = -1;
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
    if(saida_completa) {
        printf("\n----------INICIEI VELÓDROMO----------\n");
        for(int i = 0; i < 2*d; i++) {
            for(int j = 0; j < 10; j++) {
                printf("%ld ", velodromo[i][j]);
            }
            printf("\n");
        }
        printf("\n----------FINALIZEI VELÓDROMO----------\n");
    }
    */

    for(long int i = 0; i < n; i++) {
        pthread_create(&thread_ciclistas[i], NULL, thread_ciclista, &ciclistas[i+1]);
    }
    
    for(long int i = 0; i < n; i++) {
        pthread_join(thread_ciclistas[i], NULL);
    }

    for(long int i = 0; i < n; i++) {
        if(i < aux_ini) {
            printf("Quebrei :(\n");
        }
        else {
            printf("Sou o ciclista: %ld\n", vetor_final[i].identificador);
            printf("Minha colocação: %ld\n", i-aux_ini+1);
            printf("Meu instante de tempo: %ld\n", vetor_final[i].tempo_eliminacao);
        }
    }

    free(sorteados);

    return 0;
}