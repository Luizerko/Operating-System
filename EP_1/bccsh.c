#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h> 
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/socket.h>

#define MAX_CARACTER 80
#define MAX_PALAVRAS 10
#define MAX_TAMANHO_PALAVRA 20

//Printa o usuário que executou o programa e seu diretório atual.
void printa_user_dir() {
    char* usuario = getenv("USER");
    char diretorio[1000];
    getcwd(diretorio, sizeof(diretorio));

    printf("\n{%s@%s} ", usuario, diretorio);
}

//Recebe a string linha que tem palavras separadas por espaços e divide linha em varias strings, sendo cada uma delas 
//uma dessas palavras. Aloca um vetor de vetores de char (equivalente a vetor de string) chamado parsed e atribui a 
//ele as strings obtidas a partir de linha. Por fim, retorna parsed.
char** parser(char* linha) {
    char** parsed = malloc(MAX_PALAVRAS*sizeof(char*));
    for(int i = 0; i < 10; i++) {
        parsed[i] = malloc(MAX_TAMANHO_PALAVRA*sizeof(char));
        parsed[i] = NULL;
    }

    for(int i = 0; i < MAX_CARACTER; i++) {
        parsed[i] = strsep(&linha, " ");

        if(parsed[i] == NULL)
            break;
        if(parsed[i] == 0)
            i--;
    }

    return parsed;
}

//Função responsável pela execução de todos os comandos aceitos pelo bccsh.
void execucao_comandos (char** comandos, char** parseiro) {
    char* comando = parseiro[0];
    int pos;
    //Seleciona qual dos comandos foi solicitado.
    for (pos = 0; pos < 6; pos++) {
        if (strcmp(comandos[pos], comando) == 0)
            break;
    }
    //Caso correspondente ao comando "/usr/bin/du -hs .".
    if (pos == 0) {
        if(fork() == 0) {
            char* aux[] = {"/usr/bin/du", "-hs", ".", NULL};
            execve("/usr/bin/du", aux, NULL);
        }
        else {
            printf("\nExecutando o comando du:\n");
            waitpid(-1, NULL, 0);
        }
    }
    //Caso correspondente ao comando "usr/bin/traceroute www.google.com.br".
    else if (pos == 1) {
        if(fork() == 0) {
            char* aux[] = {"/usr/bin/traceroute", "www.google.com.br", NULL};
            execve("/usr/bin/traceroute", aux, NULL);
        }
        else {
            printf("\nExecutando o comando traceroute:\n");
            waitpid(-1, NULL, 0);
        }
    }
    //Caso correspondente ao comando "./ep1 x y z", sedo x um número de 1 a 3 correspondente a qual escalonador de 
    //processos será usado, y o arquivo de trace usado como entrada para a simulação e z o arquivo de saída da simulação.
    else if (pos == 2) {
        if(fork() == 0) {

            if (parseiro[4] != NULL && !strcmp(parseiro[4], "d")) {
                char* aux[] = {"./ep1", parseiro[1], parseiro[2], parseiro[3], parseiro[4], NULL};
                execve("./ep1", aux, NULL);
            }
            else {
                char* aux[] = {"./ep1", parseiro[1], parseiro[2], parseiro[3], NULL};
                execve("./ep1", aux, NULL);
            }
            
        }
        else {
            printf("\nEntrando em EP1:\n");
            waitpid(-1, NULL, 0);
        }     
    }
    //Caso correspondente ao comando "mkdir <diretorio>" que cria um diretório com nome igual a parserio[1] e modo de permissão 0777.
    else if (pos == 3) {
        mkdir(parseiro[1], 0777);
    }
    //Caso correspondente ao comando "kill -9 <PID>"" que termina o processo de PID parseiro[2]."
    else if (pos == 4) {
        kill(atoi(parseiro[2]), abs(atoi(parseiro[1])));
    }
    //Caso corresponde ao comando "ln -s <arquivo> <link>" que cria um link simbólico com nome igual a parseiro[3] de 
    //um arquivo com nome igual a parseiro[2].
    else if (pos == 5) {
        symlink(parseiro[2], parseiro[3]);
    }
    else {
        
    }
}


int main(int argc, char* argv[]) {

    char** comandos = malloc(6*sizeof(char*));
    for(int i = 0; i < 6; i++) {
        comandos[i] = malloc(100*sizeof(char));
    }
    comandos[0] = "/usr/bin/du";
    comandos[1] = "/usr/bin/traceroute";
    comandos[2] = "./ep1";
    comandos[3] = "mkdir";
    comandos[4] = "kill";
    comandos[5] = "ln";
    

    printf("---------- Seja Bem Vindo Ao BCCSH ----------\n");

    //Loop principal do shell que realiza todos os seus comandos.
    while(1) {
        printa_user_dir();
        char* linha = readline("");
        add_history(linha);
        char** parseiro = parser(linha);
        execucao_comandos(comandos, parseiro);
    }

    return 0;
}
