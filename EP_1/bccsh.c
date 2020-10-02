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


void printa_user_dir() {
    char* usuario = getenv("USER");
    char diretorio[1000];
    getcwd(diretorio, sizeof(diretorio));

    printf("\n{%s@%s} ", usuario, diretorio);
}

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

void execucao_comandos (char** comandos, char** parseiro) {
    char* comando = parseiro[0];
    int pos;
    for (pos = 0; pos < 6; pos++) {
        if (strcmp(comandos[pos], comando) == 0)
            break;
    }
    if (pos == 0) {
        //dando resultados errados mas no caminho certo.
        if(fork() == 0) {
            char* aux[] = {"/usr/bin/du", "-hs", ".", NULL};
            execve("/usr/bin/du", aux, NULL);
        }
        else {
            printf("\nExecutando o comando du:\n");
            waitpid(-1, NULL, 0);
        }
        //comandos[0] = "/usr/b"
    }
    else if (pos == 1) {
        if(fork() == 0) {
            char* aux[] = {"/usr/bin/traceroute", "www.google.com.br", NULL};
            execve("/usr/bin/traceroute", aux, NULL);
        }
        else {
            printf("\nExecutando o comando traceroute:\n");
            waitpid(-1, NULL, 0);
        }
        //comandos[1] = "/usr/bin/traceroute";
    }
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
            
        //comandos[2] = "./ep1";        
    }
    else if (pos == 3) {
        //funcionando.
        //comandos[3] = "mkdir";
        mkdir(parseiro[1], 0777);
    }
    else if (pos == 4) {
        //funcionando.
        kill(atoi(parseiro[2]), abs(atoi(parseiro[1])));
        //comandos[4] = "kill";
    }
    else if (pos == 5) {
        //funcionando.
        symlink(parseiro[2], parseiro[3]);
        //comandos[5] = "ln";
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

    while(1) {
        printa_user_dir();
        char* linha = readline("");
        add_history(linha);
        char** parseiro = parser(linha);
        execucao_comandos(comandos, parseiro);
        //break;
    }

    return 0;
}
