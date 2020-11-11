#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_CARACTER 80
#define MAX_PALAVRAS 10
#define MAX_TAMANHO_PALAVRA 20

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

int main() {
    while(1) {
        printf("[ep3]: ");
        char* linha = readline("");
        printf("%s\n", linha);
        char** parseiro = parser(linha);

        if (strcmp(parseiro[0], "mount") == 0) {
            FILE* f = fopen("simulacao/arquivo_simula"/*parseiro[1]*/, "a+"); //lembrar de trocar por parseiro[1].
            fclose(f);
        }

        else if (strcmp(parseiro[0], "cp") == 0) {
            
            FILE* f = fopen("simulacao/arquivo_simula", "a+");
            fprintf(f, "%s:\n", parseiro[1]);

            struct stat finfo;
            fstat(open(parseiro[1], O_RDONLY), &finfo);
            off_t filesize = finfo.st_size;
            fprintf(f, "    -> %ld\n", filesize);
            
            char aux[10000];
            
            FILE* origem = fopen(parseiro[1], "r+");
            FILE* destino = fopen(parseiro[2], "w+");
            
            while(fgets(aux, 10000, origem)) {
                
                fprintf(destino, "%s\n", aux);
            }
            fclose(origem);
            fclose(destino);
            fclose(f);
        }

        else if (strcmp(parseiro[0], "mkdir") == 0) {
            
        }
        
        else if (strcmp(parseiro[0], "rmdir") == 0) {
            
        }

        else if (strcmp(parseiro[0], "cat") == 0) {
            
        }

        else if (strcmp(parseiro[0], "touch") == 0) {
            
        }

        else if (strcmp(parseiro[0], "rm") == 0) {
            
        }

        else if (strcmp(parseiro[0], "ls") == 0) {
            
        }

        else if (strcmp(parseiro[0], "find") == 0) {
            
        }

        else if (strcmp(parseiro[0], "df") == 0) {
            
        }

        else if (strcmp(parseiro[0], "umount") == 0) {
            
        }

        else if (strcmp(parseiro[0], "sai") == 0) {
            
        }

    }
}