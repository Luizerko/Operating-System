#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h> 

#define MAX_CARACTER 80
#define MAX_PALAVRAS 10
#define MAX_TAMANHO_PALAVRA 20

void printa_user_dir() {
    char* usuario = getenv("USER");
    char diretorio[1000];
    getcwd(diretorio, sizeof(diretorio));

    printf("{%s@%s} ", usuario, diretorio);
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
    /*for(int i = 0; i < MAX_PALAVRAS && parseiro[i] != NULL; i++)*/
    char* comando = parseiro[0];
    int pos;
    for (pos = 0; pos < 6; pos++) {
        if (comandos[pos] == comando) {
            break;
        }
    }
    if (pos == 0) {
        //comandos[0] = "/usr/bin/du";
    }
    else if (pos == 1) {
        //comandos[1] = "/usr/bin/traceroute";
    }
    else if (pos == 2) {
        //comandos[2] = "./ep1";        
    }
    else if (pos == 3) {
        //mkdir(parseiro[1],);
        //comandos[3] = "mkdir";
    }
    else if (pos == 4) {
        //comandos[4] = "kill";
    }
    else if (pos == 5) {
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
        char** parseiro = parser(linha);
        exercucao_comandos(comandos, parseiro);
        break;
    }

    return 0;
}
