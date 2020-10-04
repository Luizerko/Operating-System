#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
#include "processo.h"
#include "time.h"

long int tempo = 1;
long int quantum = 1;
int flag_d = 0;
int cumpre_deadline = 0;

pthread_t t1;

pthread_t* t_v;
pthread_mutex_t mutex;
pthread_mutex_t* mutex_v;

//Thread utilizada no escalonador de processos 1 (First-Come First-Served). Executa o processo até que dt == 0, 
//dormindo por 1 segundo a cada iteração.
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
    pthread_exit(NULL);
}

//Recebe um min_heap processos e sua última posição indice. Muda de posição o processo processos[1] descendo-o no 
//min_heap até que esteja corretamente posicionado. A função guarda o processo da primeira posição(pai) de processos 
//e compara-o com os seus filhos. Caso o dt de processos[pai] seja menor do que pelo menos o de um de seus filhos, atribui 
//o processo filho de menor dt ao processo pai e modifica o valor de pai para o valor do índice do filho. Faz a mesma comparação 
//entre o processo[1] e os filhos do seu filho dessa vez, e assim sucessivamente. Para esse loop quando não há filhos 
//ou filhos tem valor dt maior do que processos[1]. Ao sair desse loop, atribui a processos[pai] o valor inicial de 
//processos[1]. Dessa maneira, reestabelece o min_heap.
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

//Recebe um min_heap processos e um índice filho. Muda de posição o processo processos[filho] subindo-o no min_heap 
//até que esteja corretamente posicionado. Para isso, salva numa variável aux o processo processos[filho] e compara 
//ele com o seu pai, caso exista. Se o dt de processos[pai] é maior que o de aux, atribui ao processo filho o processo
//pai dele e troca o valor de do indice filho para o indice de seu pai e o valor de pai para o para o índice pai de filho.
//Faz o mesmo processo sucessivamente até que filho <= 1 ou processos[pai]->dt <= aux->dt, quando para o loop. Por fim, 
//atribui ao processo processos[filho] o valor de aux que foi guardado no início, terminando a operação.
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

//Insere um processo novo no heap, posicionando-o inicialmente no fim do vetor e usando a função swin para reposicioná-lo
//corretamente.
void heap_insert(Processo** heap_minimo, long int indice, Processo* processo) {
    heap_minimo[indice] = processo;
    swim(indice, heap_minimo);
}

//Remove o processo que está no topo do heap. Inicialmente troca-o com o processo processos[indice-1], então usa a função 
//sink para reposicionar esse processo no min_heap corretamente.
void heap_remove(Processo** heap_minimo, long int indice) {
    heap_minimo[1] = heap_minimo[indice-1];
    sink(indice, heap_minimo);
}

//Retorna 1 se o heap está vazio e 0 no caso contrário.
int heap_empty(long int indice) {
    if (indice == 1)
        return 1;
    return 0;
}

//Thread utilizada pelo escalonador 2 (Shortest Remaining Time Next). Executa o processo arg fazendo uma iteração do 
//loop, onde tranca o semáforo da thread, faz uma operação, dorme por 1 segundo e depois destranca o semáforo do loop da 
//thread principal (thread da função main). Ao terminar o loop, a thread termina.
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

//Estrutura de nó da lista ligada curcular usada pelo escalonador Round-Robin.
typedef struct node {
    Processo* processo;
    int contador;
    struct node* prox;
} Node;

//Retorna 1 caso a lista esteja vazia e 1 caso contrário.
int lista_vazia(Node* lista_first) {
    if(lista_first == NULL || lista_first->processo == NULL)
        return 1;
    return 0;
}

//Insere um novo nó na lista ligada circular.
void lista_insere(Node** lista_first, Node** lista_last, Node* no) {
    //Caso especial para a lista vazia, onde lista_first e lista_last devem apontar para o mesmo nó.
    if (lista_vazia((*lista_first))) {
        (*lista_first) = no;
        (*lista_first)->prox = (*lista_first);
        (*lista_last) = (*lista_first);
        return;
    }
    //Caso especial para quando a lista tinha apenas um elemento, onde o próximo nó de lista_first deve ser 
    //lista_last e vice versa.
    else if (!lista_vazia((*lista_first)) && (*lista_first) == (*lista_last)) {
        (*lista_last) = no;
        (*lista_last)->prox = (*lista_first);
        (*lista_first)->prox = (*lista_last);
        return;
    }
    //Caso comum para lista que já possui 2 ou mais elementos.
    (*lista_last)->prox = no;
    no->prox = (*lista_first);
    (*lista_last) = no;
}

//Remove o primeiro elemento da lista circular.
void lista_remove(Node** lista_first, Node** lista_last) {
    //Caso especial para quando a lista só tem um elemento.
    if ((*lista_first) == (*lista_last)) {
        (*lista_first)->processo = NULL;
        return;
    }
    //Caso comum para lista com 2 ou mais elementos.
    (*lista_last)->prox = (*lista_first)->prox;
    free((*lista_first)->processo);
    free((*lista_first));
    (*lista_first) = (*lista_last)->prox;
}

//Thread utilizada pelo escalonador 3 (Round-Robin). Executa o processo arg fazendo uma iteração do loop, onde tranca 
//o semáforo da thread, faz uma operação, aumenta o contador que será comparado com o quantum em 1, dorme por 1 segundo 
//e depois destranca o semáforo do loop da thread principal (thread da função main). Quando termina o loop, a thread termina.
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

    FILE* ptr = fopen(argv[2], "r+"); //arquivo de trace

    FILE* ptr2 = fopen(argv[3], "w+"); //arquivo com resultado das simulações

    if (atoi(argv[1]) == 1) { //Caso em que é executado o escalonador First-Come First-Served.
        queueInit(); //Cria uma fila de processos.
        Processo* processo;

        while(!feof(ptr)) { //Lê o arquivo de trace e insere na fila os processos desse arquivo.
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            if(strcmp(processo->nome, ""))
                queueInsert(processo);
            
        }

        long long int tam = queueSize();
        int a;
        int flag_escreve = 1; //Flag que decide se o programa deve escrever no arquivo de resultado de simulação.
        Processo *processo1;

        //Começa a executar o primeiro processo.
        if(!queueEmpty()) {
            processo1 = queueRemove();
            if(flag_d)
                fprintf(stderr, "Novo processo: %s %d %d %d\n", processo1->nome, processo1->t0, processo1->dt, processo1->deadline);
            if(pthread_create(&t1, NULL, thread, processo1))
                printf("Dei erro 1!\n");
        }
        
        while(!queueEmpty()) {
            
            pthread_join(t1, NULL);
            if(flag_escreve) { //Ao terminar de executar um processo, escreve os seus resultados no arquivo de simulação.
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
                fprintf(ptr2, "%s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
                if (processo1->deadline >= tempo)//Checa se o processo terminou de executar antes do fim de sua deadline.
                    cumpre_deadline++;
                flag_escreve = 0;
            }
            //Se há processos na fila, o processo que estava sendo executado terminou e já deu tempo do novo processo chegar, 
            //cria a thread do novo processo e remove-o da fila.
            if(!queueEmpty() && processo1->dt == 0 && queueTop()->t0 <= tempo) {
                processo1 = queueRemove();
                flag_escreve = 1; //Como o processo terminou, atribui 1 à flag para que escreva no arquivo de resultados.
                if(flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo1->nome, processo1->t0, processo1->dt, processo1->deadline);
                if((a = pthread_create(&t1, NULL, thread, processo1)))
                    printf("Dei erro 1! Erro número: %d!\n", a);
            }
            //Caso nenhum processo novo possa ser executado, espera um segundo.
            else {
                sleep(1);
                tempo++;
            }
            
        }
        
        //Espera para que o último processo termine.
        pthread_join(t1, NULL);
        fprintf(ptr2, "%s %ld %ld\n", processo1->nome, tempo, tempo - (long int)processo1->t0);
        
        if(flag_d)
            fprintf(stderr, "Mudancas de contexto: %lld\n", tam);
        fprintf(ptr2, "%lld\n", tam);

        FILE* ptr3 = fopen("deadlines_1_1000.txt", "a+");
            fprintf(ptr3, "%d\n", cumpre_deadline);
        fclose(ptr3);
        ptr3 = fopen("contexto_1_1000.txt", "a+");
            fprintf(ptr3, "%lld\n", tam);
        fclose(ptr3);

    } 
    else if (atoi(argv[1]) == 2) { //Caso em que é executado o escalonador Shortest Remaining Time Next.
        long long int contador_contexto = 0;
        queueInit(); //Cria uma fila de processos.
        tempo = 0;
        Processo* processo;
        long int contador = 0;
        while(!feof(ptr)) { //Lê o arquivo de trace e insere na fila os processos desse arquivo.
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            
            if(strcmp(processo->nome, "")) {
                processo->id = contador;
                contador++;
                queueInsert(processo);
            }
            
        }

        t_v = malloc(queueSize()*sizeof(pthread_t)); //Malloc do vetor das threads de todos os processos.
        pthread_mutex_init(&mutex, NULL); //inicialização do semáforo do escalonador SRTN.
        mutex_v = malloc(queueSize()*sizeof(pthread_mutex_t)); //Malloc do vetor dos semáforos das threads de todos os processos.
        for(long long int i = 0; i < queueSize(); i++) {//Inicia todas as threads e tranca os semáforos de cada uma delas.
            pthread_mutex_init(&mutex_v[i], NULL);
            pthread_mutex_lock(&mutex_v[i]);
            pthread_create(&t_v[i], NULL, thread2, queueItera(i));
        }
        
        //Min heap dos processos em relação ao seu dt. O processo raiz de heap_minimo será o processo que será 
        //executado no momento, configurando o SRTN.
        Processo** heap_minimo;
        heap_minimo = malloc((queueSize()+1)*sizeof(Processo*));

        long int indice = 1;
        //heap_minimo é NULL quando não há mais processos no heap e nem processos futuros na fila. 
        while(heap_minimo != NULL) {

            if(heap_empty(indice))
                pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex);
            processo = queueTop();
            Processo* aux = heap_minimo[1];

            //Se o heap está vazio, não há nenhum processo querendo ser executado no momento. Assim, o escalonador 
            //espera um segundo para checar se novos processos chegarão.
            if(heap_empty(indice)) {
                tempo++;
                sleep(1);
            }

            //Caso em que o processo que estava sendo executado terminou. Com isso, o processo deve ser removido do 
            //heap_minimo, o que fará com que outro processo seja executado posteriormente.
            if(!heap_empty(indice) && aux->dt == 0) {
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                fprintf(ptr2, "%s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                if(aux->deadline >= tempo)
                    cumpre_deadline++;
                heap_remove(heap_minimo, indice);
                indice--;
            }
            
            //processo é o primeiro processo na fila. Se ele não é NULL (e portanto existem processos na fila) e
            //já passou tempo suficiente para que ele chegasse no sistema para ser executado (processo->to <= tempo), 
            //o processo é removido da fila e inserido no heap_minimo para que possa ser executado quando possível.
            while(processo != NULL && processo->t0 <= tempo) {
                if (flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
                queueRemove();
                heap_insert(heap_minimo, indice, processo);
                indice++; 
                processo = queueTop();
            }
            
            //Identifica o caso em que o heap_minimo[1] trocou de uma iteração para a outra do loop do escalonador e, 
            //portanto, houve mudança de contexto.
            if(!heap_empty(indice) && aux != heap_minimo[1])
                contador_contexto++;

            //Quando não há mais processos a serem executados, heap_minimo recebe NULL para que o loop termine.
            if (queueEmpty() && heap_empty(indice)) {
                free(heap_minimo);
                heap_minimo = NULL;
            }

            //Se há processo a serem executados, o escalonador libera o semáforo da raiz de heap_minimo, permitindo a execução do processo.
            if(!heap_empty(indice))
                pthread_mutex_unlock(&mutex_v[heap_minimo[1]->id]);

        }

        //Espera até que todos os processos tenham terminado.
        for (int i = 0; i < queueSize(); i++) {
            pthread_join(t_v[i], NULL);
        }

        if(flag_d)
            fprintf(stderr, "Mundancas de contexto: %lld\n", contador_contexto);

        fprintf(ptr2, "%lld\n", contador_contexto);

        FILE* ptr3 = fopen("deadlines_2_1000.txt", "a+");
            fprintf(ptr3, "%d\n", cumpre_deadline);
        fclose(ptr3);
        ptr3 = fopen("contexto_2_1000.txt", "a+");
            fprintf(ptr3, "%lld\n", contador_contexto);
        fclose(ptr3);

    } 
    else { //Caso em que é executado o escalonador Round-Robin.
        long long int contador_contexto = 0;
        queueInit(); //Cria uma fila de processos.
        tempo = 0;
        Processo* processo;
        long int contador = 0;

        while(!feof(ptr)) { //Lê o arquivo de trace e insere na fila de processos os processos desse arquivo.
            processo = malloc(sizeof(Processo));
            
            fscanf(ptr, "%s %d %d %d", processo->nome, &(processo->t0), &(processo->dt), &(processo->deadline));
            
            if(strcmp(processo->nome, "")) {
                processo->id = contador;
                contador++;
                queueInsert(processo);
            }
            
        }

        //Cria um nó para cada processo da fila.
        Node** nos = malloc(queueSize()*sizeof(Node*));
        for(int i = 0; i < queueSize(); i++) {
            nos[i] = malloc(sizeof(Node));
        }

        t_v = malloc(queueSize()*sizeof(pthread_t)); //Malloc do vetor das threads de todos os processos.
        pthread_mutex_init(&mutex, NULL); //inicialização do semáforo do escalonador Round-Robin.
        mutex_v = malloc(queueSize()*sizeof(pthread_mutex_t)); //Malloc do vetor dos semáforos das threads de todos os processos.
        for(long long int i = 0; i < queueSize(); i++) { //Inicia todas as threads e tranca os semáforos de cada uma delas.
            pthread_mutex_init(&mutex_v[i], NULL);
            pthread_mutex_lock(&mutex_v[i]);
            nos[i]->processo = queueItera(i); //Retorna, caso exista, o processo que está na posição i da fila.
            nos[i]->contador = 0;
            nos[i]->prox = NULL;
            pthread_create(&t_v[i], NULL, thread3, nos[i]);
        }
        
        //Primeira posição da fila.
        Node* lista_first = malloc(sizeof(Node));
        lista_first->processo = NULL;
        lista_first->prox = NULL;

        Node* lista_last = lista_first; //No inicio, o apontador para o último nó da fila e o apontador para o primeiro nó apontam para o mesmo nó.
        
        while(lista_first != NULL) { //Quando lista_first é NULL, não há mais processos para serem executados.
            
            //Se a lista está vazia, o loop do escalonador deve continuar a rodar, para que se passe o tempo em espera de novos processos.
            if(lista_vazia(lista_first))
                pthread_mutex_unlock(&mutex);
            
            pthread_mutex_lock(&mutex);
            processo = queueTop(); //Proximo processo que chegará no sistema.
            Processo* auxiliar = lista_first->processo; //Primeiro processo da fila.
            
            //Se a lista está vazia, passa o tempo.
            if(lista_vazia(lista_first)) {
                tempo++;
                sleep(1);
            }

            //Se a lista não está vazia e o processo que estava na frente da fila terminou de rodar, esse processo deve ser removido da fila. Caso existam 2 ou mais processos, 
            //o processo que estava em segundo lugar na fila passa para primeiro, para que possa ser executado.
            if(!lista_vazia(lista_first) && lista_first->processo->dt == 0) {
                Processo* aux = lista_first->processo;
                if(flag_d)
                    fprintf(stderr, "Finalizacao do processo: %s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                fprintf(ptr2, "%s %ld %ld\n", aux->nome, tempo, tempo - (long int)aux->t0);
                if(aux->deadline >= tempo)
                    cumpre_deadline++;
                lista_remove(&lista_first, &lista_last);
            }

            //Se a lista não está vazia e o processo que está em primeiro lugar executou pelo menos o mesmo tempo que o valor de quantum, esse processo é posto em último lugar 
            //da fila e o processo que estava em segundo lugar passa para primeiro e é executado.
            if(!lista_vazia(lista_first) && lista_first->contador >= quantum) {
                lista_first->contador = 0;
                if(lista_first != lista_last) {
                    lista_first = lista_first->prox;
                    lista_last = lista_last->prox;
                }
            }

            //Retira da fila todos os processos com (t0 <= tempo) que, portanto, já chegaram ao sistema e os coloca na lista circular na última posição.
            while(processo != NULL && processo->t0 <= tempo) {
                if (flag_d)
                    fprintf(stderr, "Novo processo: %s %d %d %d\n", processo->nome, processo->t0, processo->dt, processo->deadline);
                queueRemove();
                lista_insere(&lista_first, &lista_last, nos[processo->id]);
                processo = queueTop();
            }

            //Caso em que não há procesos para serem executados e nenhum processo novo chegará. Assim, o escalonador sai do loop e termina.
            if(queueEmpty() && lista_vazia(lista_first)) {
                free(lista_first);
                lista_first = NULL;
            }

            //Se há processos na lista o semáforo do processo na primeira posição é destravado. Além disso, se o processo que estava em primeiro lugar foi trocado por outro, 
            //houve mudança de contexto.
            if(!lista_vazia(lista_first)) {
                if(auxiliar != lista_first->processo)
                    contador_contexto++;
                pthread_mutex_unlock(&mutex_v[lista_first->processo->id]);
            }
        }

        for (int i = 0; i < queueSize(); i++) { //Espera até que todos os processos terminem.
            pthread_join(t_v[i], NULL);
        }

        if(flag_d)
            fprintf(stderr, "Mundancas de contexto: %lld\n", contador_contexto);

        fprintf(ptr2, "%lld\n", contador_contexto);

        FILE* ptr3 = fopen("deadlines_3_1000.txt", "a+");
            fprintf(ptr3, "%d\n", cumpre_deadline);
        fclose(ptr3);
        ptr3 = fopen("contexto_3_1000.txt", "a+");
            fprintf(ptr3, "%lld\n", contador_contexto);
        fclose(ptr3);

    }

    fclose(ptr);
    fclose(ptr2);

    return 0;
}