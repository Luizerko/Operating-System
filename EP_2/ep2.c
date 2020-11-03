#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


//Estrutura que guarda 2 inteiros correspondendo à coordenada em relação ao eixo x e ao eixo y.
typedef struct coordenada {
    long int x;
    long int y;
} Coordenada;


//Estrutura que guarda um ciclitas com os dados de seu identificador, sua posição em relação à árvore do vetor arrive, o tipo de nó me relação à árvore (raiz, interno ou folha), 
//a volta em que o ciclista está, a sua velocidade atual e as coordenadas que correspondem à posição da sua parte dianteira.
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


//Estrutura que guarda os dados em relação à finalização da corrida de um ciclista. contém seu identificador, seu rank na corrida (caso não tenha sido eliminado por quebrar), 
//o momento em que ele foi eliminado e a votlta em que ele quebrou (caso tenha quebrado).

typedef struct final_ciclista {
    long int identificador;
    long int rank;
    long int tempo_eliminacao;
    long int volta_quebra;
} Final_Ciclista;

_Atomic long int** velodromo; //Velodromo onde acontece a corrida representando 1m em relação a y e0,5m em relação a x por posição.
long int ** pista; //Velodromo ajustado para d posições. 
pthread_mutex_t mutex; //Mutex que impede que a instrução de movimento de ciclistas diferentes rode ao mesmo tempo.
pthread_t* thread_ciclistas;
Ciclista* ciclistas;
int* arrive; //Vetor de arrive que indica se um ciclista já fez sua instrução nessa iteração de acordo com sua posicao_arvore.

long int d, n; //Número de posições verticais do velódromo e número de ciclistas da corrida. 
long int tempo = 0; //Duração atual da corrida.
int saida_completa = 0;
long int* voltas_eliminacao; //Vetor utilizando para saber quando um ciclista foi o último a completar uma volta e, portanto, deve ser elimilado.
Final_Ciclista* vetor_final; //Vetor com as informações finais de todos os ciclistas no fim da corrida.
long int aux_ini, aux_fim;
long int tamanho; //Quantidade atual de ciclistas na corrida.
int continua; //Variável utilizada pela raiz da árvore de ciclistas para que todos rodem sua iteração antes que qualquer um deles possa rodar novamente.
long int ultimo_arvore; // Cilcista que está na última posição da árvore
long int tempo_cruzamento; //Tempo em que o ciclista que ganha a corrida cruzou a linha de chegada pela última vez.

long int noventa_por_hora = -1; //Flag que identifica qual corredor andará a 90km/h nas duas últimas voltas caso algum deles vá andar nessa velocidade.
int flag_noventa; //Flag que identifica se já foi calculado se um corredor andará a 90km/h nas últimas duas voltas.

int mod(long int num, long int d) {
    if (num < 0)
        num = d + num;
    return num%d; 
}

//Checa se um ciclista está na mesma coordenada em relação a x do biker e na coordenada aux_y em relação a y, retornando 1 caso sim e - caso não.
int lado(Ciclista* biker, int aux_y) {
    if(velodromo[biker->coordenada.x][aux_y] != -1 && ((velodromo[biker->coordenada.x][aux_y] == velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y]) ||
        (velodromo[biker->coordenada.x][aux_y] != velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] && velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != -1)) )
        return 1;
    return 0;
}

//Checa se um ciclista está uma posição atras em relação à coordenada x do biker e na coordenada aux_y em relação a y, retornando 1 caso sim e - caso não.
int lado_mais_um(Ciclista* biker, int aux_y) {
    if(velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != -1 && ((velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] == velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y]) ||
        (velodromo[mod(biker->coordenada.x+1, 2*d)][aux_y] != velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y] && velodromo[mod(biker->coordenada.x+2, 2*d)][aux_y] != -1)) )
        return 1;
    return 0;
}


//Instrução de movimento dos ciclistas.
void instrucao(Ciclista* biker) {
    //Coordenada da posição do cicista antes de ele se movimentar nessa iteração.
    Coordenada coord_anterior;
    coord_anterior.x = biker->coordenada.x;
    coord_anterior.y = biker->coordenada.y;

    if(biker->volta < 1) {//Velocidade é 30km/h para todos os ciclistas.
        if(biker->coordenada.x - 1 < 0)
            biker->volta++;
        biker->coordenada.x = mod(biker->coordenada.x-1, 2*d); //Anda 0,5m para frente no velódromo.

        //Apaga no velodromo o identificador do ciclista na posição que ele estava e escreve na nova posição dele seu identificador.
        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;
        if(velodromo[mod((coord_anterior.x+1), 2*d)][coord_anterior.y] == biker->identificador)
            velodromo[mod((coord_anterior.x+1), 2*d)][coord_anterior.y] = -1;

        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = biker->identificador;

    }

    else if(biker->volta < (2*n) - 2 || tamanho > 2) { //Voltas intermediárias.
        
        if(biker->velocidade == 30) { //Movimentação para ciclistas com velocidade de 30km/h.

            biker->coordenada.x = mod(biker->coordenada.x-1, 2*d); //Anda 0,5m para frente no velódromo.
            int aux_y = 0;
            
            //Analisa se é possivel para o ciclista se posicionar mais à esquerda no velódromo, modificando sua coordenada em relação à y se possível.
            while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                    (lado_mais_um(biker, aux_y)) ||
                    (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                aux_y++;
            }
            
            biker->coordenada.y = aux_y;
        }
        else if(biker->velocidade == 60) { //Movimentação para ciclistas com velocidade de 60km/h.
            int aux_y = biker->coordenada.y;
            
            //Checa se é possível para o ciclista andar a 60km/h para frente sem se chocar com outro ciclista
            if((velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y] == -1 || (ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].velocidade == 60 && !arrive[ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].posicao_arvore])) && 
                (velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y] == -1 || !arrive[ciclistas[velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y]].posicao_arvore])) {
                
                biker->coordenada.x = mod(biker->coordenada.x-2, 2*d); //Anda 1m para frente no velódromo.
                
                //Checa se é possível para o ciclista se posicionar mais à esquerda no velodromo, modificando sua coordenada em relação à y se possível.
                aux_y = 0;
                while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                biker->coordenada.y = aux_y;

            }
            else { //Caso em que o ciclista não conseguiu andar para frente a 60km/h
            
                biker->coordenada.x = mod(biker->coordenada.x-2, 2*d); //Anda 1m para frente no velódromo.
                
                //Checa se ele consegue andar a 60km/h para para frente e para a direita em alguma posição, modificando aux_y ao tentar outra posição
                while(aux_y < 10 && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }

                
                if(aux_y == 10) {//Anda a 30 km/h nessa iteração.
                    //Corrige a coordenada por ter andando a 30km/h e não a 60.
                    biker->coordenada.x = mod((biker->coordenada.x + 1), 2*d);
                    
                    //Analisa a primeira posição da esquerda apra direita em que o ciclista pode se posicionar no velódormo.
                    aux_y = 0;
                    while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                        aux_y++;
                    }
                }
    
                //Atribui a nova coordenada do ciclista em relação a y.
                biker->coordenada.y = aux_y;

            }
        }

        //Apaga a antiga posição do ciclista no velódromo e escreve o identificador na nova coordenada.
        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;
        if(velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] == biker->identificador)
            velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] = -1;
        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = biker->identificador;
        

        if(coord_anterior.x < biker->coordenada.x) { //Checa que uma volta se passou.
            biker->volta++;
            if(biker->volta%2 != 0) {
                if(voltas_eliminacao[biker->volta] == tamanho - 1) {//Remoção deste ciclista da corrida no caso em que ele foi o último a completar essa volta.
                    tamanho--; //Diminui o número de iclistas na corrida.
                    vetor_final[aux_fim].identificador = biker->identificador;
                    vetor_final[aux_fim].tempo_eliminacao = tempo + 60; //Elimina com uma passagem de tempo extra, já que considera a passagem de tempo da iteração atual em que atingiu a nova volta e foi eliminado.
                    aux_fim--;
                    
                    //Tira do velodoromo o identificador od ciclista da posição em que estava, já que foi eliminado.
                    velodromo[biker->coordenada.x][biker->coordenada.y] = -1;
                    velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = -1;
                    
                    int flag_raiz = 0;
                    if(biker->posicao_arvore == 1)//É a raiz.
                        flag_raiz = 1; 
                    
                    //Troca o arrive do ultimo ciclista da ármore pelo que está sendo eliminado e, depois, troca a posição deles na árvore.
                    arrive[biker->posicao_arvore] = arrive[ciclistas[ultimo_arvore].posicao_arvore];
                    ciclistas[ultimo_arvore].posicao_arvore = biker->posicao_arvore;
                    biker->posicao_arvore = -1; //Atribui -1 a posição da árvore removida para que não gere problemas posteriormente.

                    //Acha o identificador do ciclsita que está na nova última posição da árvore.
                    for(long int w = 1; w <= n; w++) {
                        if(ciclistas[w].posicao_arvore == tamanho) {
                            ultimo_arvore = w;
                            break;
                        }
                    }

                    //Se o ciclista é a raíz, ajeita os valores da árvore arrive e do continua.
                    if(flag_raiz) {
                        for(int j = 1; j <= tamanho+1; j++)
                            arrive[j] = 0;
                        continua = 1 - continua;
                    }
                    
                    //Destrava mutex e finaliza a thread.
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }
                else //Caso o ciclista não seja o último a chegar na volta, conta que mais um ciclista passou por essa volta.
                    voltas_eliminacao[biker->volta]++;
            }

            if (biker->volta%6 == 0 && tamanho > 5) { //Checa se ciclistas podem ser removidos da prova por quebrar nessa iteração.
                if(rand()%100 < 5) { //Decide se um ciclista quebra.
                    tamanho--; //Diminui o número de iclistas na corrida.
                    vetor_final[aux_fim].identificador = biker->identificador;
                    vetor_final[aux_fim].tempo_eliminacao = tempo + 60; //Elimina com uma passagem de tempo extra, já que considera a passagem de tempo da iteração atual em que atingiu a nova volta e foi eliminado.
                    aux_fim--;
                    
                    //Tira do velodoromo o identificador od ciclista da posição em que estava, já que foi eliminado
                    velodromo[biker->coordenada.x][biker->coordenada.y] = -1;
                    velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = -1;
                    
                    int flag_raiz = 0;
                    if(biker->posicao_arvore == 1)//É a raiz.
                        flag_raiz = 1; 
                    
                    //Troca o arrive do ultimo ciclista da ármore pelo que está sendo eliminado e, depois, troca a posição deles na árvore.
                    arrive[biker->posicao_arvore] = arrive[ciclistas[ultimo_arvore].posicao_arvore];
                    ciclistas[ultimo_arvore].posicao_arvore = biker->posicao_arvore;
                    biker->posicao_arvore = -1; //Atribui -1 a posição da árvore removida para que não gere problemas posteriormente.

                    //Acha o identificador do ciclsita que está na nova última posição da árvore.
                    for(long int w = 1; w <= n; w++) {
                        if(ciclistas[w].posicao_arvore == tamanho) {
                            ultimo_arvore = w;
                            break;
                        }
                    }

                    //Se o ciclista é a raíz, ajeita os valores da árvore arrive e do continua.
                    if(flag_raiz) {
                        for(int j = 1; j <= tamanho+1; j++)
                            arrive[j] = 0;
                        continua = 1 - continua;
                    }
                    
                    //Destrava mutex e finaliza a thread.
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }
            }
            
            //Trocas de velocidade do ciclista no caso de ele não ter sido removido.
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

    else { //Duas últimas voltas
        //Calcula se um ciclista vai andar a 90km/h nas duas  últimas voltas e qual deles andará nessa velocidade.
        if(flag_noventa != 1) {
            if(rand()%1 == 0) // TROCAR ISSO DEPOIS PARA rand()%10 == 0
                noventa_por_hora = rand()%2;                
            flag_noventa = 1;
        }
        
        //Modifica a velocidade do ciclista caso ele tenha sido selecionado.
        if(noventa_por_hora == 0) {
            biker->velocidade = 90;
            noventa_por_hora = -10;
        }
        noventa_por_hora = 1 - noventa_por_hora;
        
        if(biker->velocidade == 30) { //Movimentação para ciclistas com velocidade de 30km/h.

            biker->coordenada.x = mod(biker->coordenada.x-1, 2*d); //Anda 0,5m para frente no velódromo.
            int aux_y = 0;
            
            //Analisa se é possivel para o ciclista se posicionar mais à esquerda no velódromo, modificando sua coordenada em relação à y se possível.
            while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                    (lado_mais_um(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade != 90) ||
                    (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                aux_y++;
            }
            
            biker->coordenada.y = aux_y;
        }
        else if(biker->velocidade == 60) {
            int aux_y = biker->coordenada.y;
            
            //Checa se é possível para o ciclista andar a 60km/h para frente sem se chocar com outro ciclista
            if((velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y] == -1 || (ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].velocidade != 30 && !arrive[ciclistas[velodromo[mod(biker->coordenada.x-1, 2*d)][aux_y]].posicao_arvore])) && 
              (velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y] == -1 || !arrive[ciclistas[velodromo[mod(biker->coordenada.x-2, 2*d)][aux_y]].posicao_arvore])) {
                    
                biker->coordenada.x = mod(biker->coordenada.x-2, 2*d); //Anda 1m para frente no velódromo.
                
                //Checa se é possível para o ciclista se posicionar mais à esquerda no velodromo, modificando sua coordenada em relação à y se possível.
                aux_y = 0;
                while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)  && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade != 90) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
                biker->coordenada.y = aux_y;

            }
            else { //Caso em que o ciclista não conseguiu andar para frente a 60km/h
            
                biker->coordenada.x = mod(biker->coordenada.x-2, 2*d); //Anda 1m para frente no velódromo.
                
                //Checa se ele consegue andar a 60km/h para para frente e para a direita em alguma posição, modificando aux_y ao tentar outra posição
                while(aux_y < 10 && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                        (lado_mais_um(biker, aux_y)  && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade != 90) ||
                        (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                    aux_y++;
                }
    
                //Atribui a nova coordenada do ciclista em relação a y.
                biker->coordenada.y = aux_y;

            }
        }

        else {
            int aux_y = biker->coordenada.y;
                
            biker->coordenada.x = mod(biker->coordenada.x-3, 2*d); //Anda 1,5m para frnete no velódromo
            
            aux_y = 0;

            //Checa se é possível para o ciclista se posicionar mais à esquerda no velodromo, modificando sua coordenada em relação à y se possível.
            while(aux_y < biker->coordenada.y && ((lado(biker, aux_y) && ciclistas[velodromo[biker->coordenada.x][aux_y]].velocidade == 30) ||
                    (lado_mais_um(biker, aux_y)) ||
                    (velodromo[biker->coordenada.x][aux_y] != -1 && arrive[ciclistas[velodromo[biker->coordenada.x][aux_y]].posicao_arvore]))) {
                aux_y++;
            }
            biker->coordenada.y = aux_y;
        }

        //Apaga a antiga posição do ciclista no velódromo e escreve o identificador na nova coordenada.
        if(velodromo[coord_anterior.x][coord_anterior.y] == biker->identificador)
            velodromo[coord_anterior.x][coord_anterior.y] = -1;
        if(velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] == biker->identificador)
            velodromo[mod(coord_anterior.x+1, 2*d)][coord_anterior.y] = -1;
        velodromo[biker->coordenada.x][biker->coordenada.y] = biker->identificador;
        velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = biker->identificador;

        if(coord_anterior.x < biker->coordenada.x) { //Checa que uma volta se passou.
            biker->volta++;

            if(biker->volta%2 != 0) {
                if(voltas_eliminacao[biker->volta] == tamanho - 1) {//Remoção deste ciclista da corrida no caso em que ele foi o último a completar essa volta.
                    tamanho--; //Diminui o número de iclistas na corrida.
                    vetor_final[aux_fim].identificador = biker->identificador;
                    vetor_final[aux_fim].tempo_eliminacao = tempo+60; //Elimina com uma passagem de tempo extra, já que considera a passagem de tempo da iteração atual em que atingiu a nova volta e foi eliminado.
                    aux_fim--;
                    
                    //Tira do velodoromo o identificador od ciclista da posição em que estava, já que foi eliminado.
                    velodromo[biker->coordenada.x][biker->coordenada.y] = -1;
                    velodromo[mod(biker->coordenada.x+1, 2*d)][biker->coordenada.y] = -1;

                    int flag_raiz = 0;
                    if(biker->posicao_arvore == 1)//É a raiz.
                        flag_raiz = 1; 

                    //Troca o arrive do ultimo ciclista da ármore pelo que está sendo eliminado e, depois, troca a posição deles na árvore.
                    arrive[biker->posicao_arvore] = arrive[ciclistas[ultimo_arvore].posicao_arvore];
                    ciclistas[ultimo_arvore].posicao_arvore = biker->posicao_arvore;
                    biker->posicao_arvore = -1; //Atribui -1 a posição da árvore removida para que não gere problemas posteriormente.

                    //Acha o identificador do ciclsita que está na nova última posição da árvore.
                    for(long int w = 1; w <= n; w++) {
                        if(ciclistas[w].posicao_arvore == tamanho) {
                            ultimo_arvore = w;
                            break;
                        }
                    }

                    //Se o ciclista é a raíz, ajeita os valores da árvore arrive e do continua.
                    if(flag_raiz) {
                        for(int j = 1; j <= tamanho+1; j++)
                            arrive[j] = 0;
                        continua = 1 - continua;
                    }   
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(NULL);
                }
                //Caso o ciclista não seja o último a chegar na volta, conta que mais um ciclista passou por essa volta.
                else {
                    voltas_eliminacao[biker->volta]++;
                    tempo_cruzamento = tempo+60;
                }
            }

            //Trocas de velocidade do ciclista no caso de ele não ter sido removido.
            if(biker->velocidade == 30) {
                if(rand()%10 < 8)
                    biker->velocidade = 60;
            }
            else if (biker->velocidade == 60) {
                if(rand()%10 < 4)
                    biker->velocidade = 30;
            }
        }

    }

    arrive[biker->posicao_arvore] = 1;
}

void* thread_ciclista(void* arg) {
    int round = 0; //Variável usada para garantir, junto com o continua, que o ciclista não vai rodar sua iteração novamente antes que todos os cilcistas andem.
    Ciclista* biker = (Ciclista*)arg;

    while(1) {

        printf(" menino %ld, velocidade %ld e tô na volta: %ld\n", biker->identificador, biker->velocidade, biker->volta);

        //Caso em que só sobrou um ciclista na corrida, terminando sua execução.
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

            pthread_mutex_unlock(&mutex);
            
            //Espera que a raiz mude o calor de continua.
            if (round == 0) {
                while (continua == 0) {
                    usleep(d*10*(5-1));
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*10*(5-1));
                }
                round = 0;                
            }
        }

        else if (biker->tipoNo == 2) { //Interno
            
            //Espera até que seus filhos na árvore rodem ou não existam.
            while(2*biker->posicao_arvore <= tamanho && arrive[2*biker->posicao_arvore] == 0) {
                usleep(d*10*(5-1));
            }

            while(2*biker->posicao_arvore + 1 <= tamanho && arrive[2*biker->posicao_arvore + 1] == 0) {
                usleep(d*10*(5-1));
            }

            pthread_mutex_lock(&mutex);
            instrucao(biker);

            pthread_mutex_unlock(&mutex);

            //Espera que a raiz mude o calor de continua.
            if (round == 0) {
                while (continua == 0) {
                    usleep(d*10*(5-1));
                }
                round = 1;
            }
            else {
                while (continua == 1) {
                    usleep(d*10*(5-1));
                }
                round = 0;                
            }
        }

        else { //Raiz

            //Espera até que seus filhos na árvore rodem ou não existam.
            while(2*biker->posicao_arvore <= tamanho && arrive[2*biker->posicao_arvore] == 0) {
                usleep(d*40*(5-1));
            }

            while(2*biker->posicao_arvore + 1 <= tamanho && arrive[2*biker->posicao_arvore + 1] == 0) {
                usleep(d*40*(5-1));
            }

            pthread_mutex_lock(&mutex);
            instrucao(biker);
            tempo += 60;
            
            if(saida_completa) {

                for(int i = 0; i < 2*d; i++) {
                    for(int j = 0; j < 10; j++) {
                        pista[i/2][j] = velodromo[i][j];
                    }
                }
                
                /*
                if(noventa_por_hora != -1 && noventa_por_hora != 2) {
                    int printa_20_ms = 2;
                    while(printa_20_ms >= 0) {
                        printf("\n---------- INICIEI VELÓDROMO (20) ----------\n");
                        
                        if(printa_20_ms == 2) {
                            for(int i = 0; i < d; i++) {
                                for(int j = 0; j < 10; j++) {
                                    if(pista[i][j] != -1) {
                                        if(pista[mod(i+1, d)][j] != -1) {
                                            pista[mod(i+2, d)][j] = pista[mod(i+1, d)][j];
                                            pista[mod(i+1, d)][j] = pista[i][j];
                                            pista[i][j] = -1;
                                            j = 10;
                                            i = d;
                                        }
                                        else {
                                            pista[mod(i+1, d)][j] = pista[i][j];
                                            pista[i][j] = -1;
                                        }
                                    }
                                }
                            }
                        }
                        
                        else if(printa_20_ms == 1){
                            for(int i = 0; i < d; i++) {
                                for(int j = 0; j < 10; j++) {
                                    if(pista[i][j] != -1 && ciclistas[pista[i][j]].velocidade == 90) {
                                        if(pista[mod(i-1, d)][j] != -1) {
                                            j = 10;
                                            i = d;
                                        }
                                        else {
                                            pista[mod(i-1, d)][j] = pista[i][j];
                                            pista[i][j] = -1;
                                        }
                                    }
                                }
                            }
                        }
                        
                        else {
                            for(int i = 0; i < d; i++) {
                                for(int j = 0; j < 10; j++) {
                                    if(pista[i][j] != -1 && ciclistas[pista[i][j]].velocidade == 90) {
                                        if(pista[mod(i-1, d)][j] != -1) {
                                            pista[mod(i-2, d)][j] = pista[mod(i-1, d)][j];
                                            pista[mod(i-1, d)][j] = pista[i][j];
                                            pista[i][j] = -1;
                                            j = 10;
                                            i = d;
                                        }
                                    }
                                    else if(pista[i][j] != -1 && ciclistas[pista[i][j]].velocidade != 90) {
                                        pista[mod(i-1, d)][j] = pista[i][j];
                                        pista[i][j] = -1;
                                    }
                                }
                            }
                        }
                        
                        for(int i = 0; i < d; i++) {
                            for(int j = 0; j < 10; j++) {
                                printf("%ld ", pista[i][j]);
                            }
                            printf("\n");
                        }
                        /*
                        for(int i = 0; i < 2*d; i++) {
                            for(int j = 0; j < 10; j++) {
                                printf("%ld ", velodromo[i][j]);
                            }
                            printf("\n");
                        }
                        
                       
                        printf("\n---------- FINALIZEI VELÓDROMO (20) ----------\n");
                        printa_20_ms--;
                    }
                    getchar();
                }
                */

                printf("\n---------- INICIEI VELÓDROMO ----------\n");
                for(int i = 0; i < d; i++) {
                    for(int j = 0; j < 10; j++) {
                        printf("%ld ", pista[i][j]);
                    }
                    printf("\n");
                }  
                /*
                for(int i = 0; i < 2*d; i++) {
                    for(int j = 0; j < 10; j++) {
                        printf("%ld ", velodromo[i][j]);
                    }
                    printf("\n");
                }
                */
                printf("\n---------- FINALIZEI VELÓDROMO ----------\n");
                
            }

            //Zera os arrives de todos os ciclsitas da árvore. 
            for (int i = 1; i <= n; i++) {
                arrive[i] = 0;
            }
            pthread_mutex_unlock(&mutex);
            //Muda o valor de continua e arrive para que todos os cilcistas possam executar suas iterações novamente.
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

    clock_t start = clock();

    d = (long int)atoi(argv[1]); //Recebe o tamanho do velódromo.
    n = (long int)atoi(argv[2]); //Recebe o númeor de ciclistas.
    if(argc > 3) { //Flag de debug.
        saida_completa = 1;
    }
    tamanho = n;
    ultimo_arvore = n;
    pthread_mutex_init(&mutex, NULL);
    srand(time(NULL));

    ciclistas = malloc((n+1)*sizeof(Ciclista));
    thread_ciclistas = malloc(n*sizeof(pthread_t));
    velodromo = malloc(2*d*sizeof(long int*)); 
    for(long int i = 0; i < 2*d; i++) {
        velodromo[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            velodromo[i][j] = -1;    
        }
    }
    pista = malloc(d*sizeof(long int*));
    for(long int i = 0; i < d; i++) {
        pista[i] = malloc(10*sizeof(long int));
        for(int j = 0; j < 10; j++) {
            pista[i][j] = -1;    
        }
    }
    voltas_eliminacao = malloc(20*n*sizeof(long int));
    for(long int i = 0; i < 20*n; i++) {
        voltas_eliminacao[i] = 0;
    }
    aux_fim = n-1;
    aux_ini = 0;
    vetor_final = malloc(n*sizeof(Final_Ciclista));
    for (long int i = 0; i < n; i++) {
        vetor_final[i].volta_quebra = -1;
    }

    //Dados iniciais de todos os cilcistas.
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

    //Sorteia qual ciclista ficará em cada posição, no inicio da corrida, escrevendo eles em suas respectivas posições.
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

    //Cria as threads de todos os ciclistas.
    for(long int i = 0; i < n; i++) {
        pthread_create(&thread_ciclistas[i], NULL, thread_ciclista, &ciclistas[i+1]);
    }
    
    //Espera até que todos so ciclistas terminem de executar.
    for(long int i = 0; i < n; i++) {
        pthread_join(thread_ciclistas[i], NULL);
    }

    //Atribui o tempo em que o ciclista que ficou em primeior lugar cruzou a linha de chegada pela última vez.
    vetor_final[aux_ini].tempo_eliminacao = tempo_cruzamento;

    //Printa os resultados finais da corrida.
    for(long int i = 0; i < n; i++) {
        if(i < aux_ini) {
            printf("\n");
            printf("Sou o ciclista: %ld\n", vetor_final[i].identificador);
            printf("Quebrei na volta %ld :(\n", vetor_final[i].volta_quebra);
            printf("\n");
        }
        else {
            printf("\n");
            printf("Sou o ciclista: %ld\n", vetor_final[i].identificador);
            printf("Minha colocação: %ld\n", i-aux_ini+1);
            printf("Meu instante de tempo: %ld\n", vetor_final[i].tempo_eliminacao);
            printf("\n");
        }
    }

    free(sorteados);
    for(long int i = 0; i < 2*d; i++)
        free(velodromo[i]);
    free(velodromo);
    for(long int i = 0; i < d; i++)
        free(pista[i]);
    free(pista);
    free(ciclistas);
    free(thread_ciclistas);
    free(voltas_eliminacao);
    free(vetor_final);
    free(arrive);

    clock_t end = clock();
    double time_computed = (double)(end - start)/CLOCKS_PER_SEC;
    
    FILE* ptr;
    
    if(d <= 250) {
        if(n <= 5) {
            ptr = fopen("grafico_pista_pequena_poucos.txt", "a+");
        }
        else if (n <= 20) {
            ptr = fopen("grafico_pista_pequena_medios.txt", "a+");
        }
        else {
            ptr = fopen("grafico_pista_pequena_muitos.txt", "a+");
        }
    }
    else if(d <= 500) {
        if(n <= 5) {
            ptr = fopen("grafico_pista_media_poucos.txt", "a+");
        }
        else if (n <= 20) {
            ptr = fopen("grafico_pista_media_medios.txt", "a+");
        }
        else {
            ptr = fopen("grafico_pista_media_muitos.txt", "a+");
        }
    }
    else {
        if(n <= 5) {
            ptr = fopen("grafico_pista_grande_poucos.txt", "a+");
        }
        else if (n <= 20) {
            ptr = fopen("grafico_pista_grande_medios.txt", "a+");
        }
        else {
            ptr = fopen("grafico_pista_grande_muitos.txt", "a+");
        }
    }

    fprintf(ptr, "%ld %lf\n", n, time_computed);
    
    fclose(ptr);

    return 0;
}