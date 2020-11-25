#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>


#define MAX_CARACTER 80
#define MAX_PALAVRAS 10
#define MAX_TAMANHO_PALAVRA 20

char sistema_de_arquivos[200];

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
    strcpy(sistema_de_arquivos, "/home/lui/Documents/IME/2020.2/SO/SO_rep/EP_3/simulacao/arquivo_simula");
    while(1) {
        printf("[ep3]: ");
        char* linha = readline("");
        printf("%s\n", linha);
        char** parseiro = parser(linha);

        if (strcmp(parseiro[0], "mount") == 0) {
            /* LEMBRAR DE TROCAR PRA ISSO DEPOIS
            for(int i = 0; i < strlen(parseiro[1]); i++) {
                sistema_de_arquivos[i] = parseiro[1][i];
            }
            sistema_de_arquivos[strlen(parseiro[1])] = '\0';
            */
            if(access("simulacao/arquivo_simula", F_OK) == -1) {
                FILE* f = fopen("simulacao/arquivo_simula"/*parseiro[1]*/, "a+"); //lembrar de trocar por parseiro[1].
                //FAT começa a contar da posição 1, ou seja, a primeira posição do FAT indica linha 1 e não linha 0. Caso tenha um valor 0 numa posição do FAT, significa que essa posição está vazia.
                for(int i = 0; i < 25600; i++) {
                    fprintf(f, "0                   |");
                }

                fprintf(f, "\n");
                for(int i = 0; i < 25600; i++) {
                    fprintf(f, "0");
                }
                fprintf(f, "\n");
                /*
                for(int i = 0; i < 25600; i++) {
                    fprintf(f, "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \n");
                }
                */
                fclose(f);
            }
        }

        else if (strcmp(parseiro[0], "cp") == 0) {
            
            FILE* f = fopen("simulacao/arquivo_simula", "a+");

            struct stat finfo;
            fstat(open(parseiro[1], O_RDONLY), &finfo);
            off_t filesize = finfo.st_size;
            fclose(f);

            int posicao_blocos_ocupados = 0;
            int blocos_ocupados[25600];

            long int blocos = (long int)ceil((float)filesize/(float)4096);
            //printf("%ld\n", blocos);

            f = fopen("simulacao/arquivo_simula", "r+");
            char memoria[21];
            int indice = 0;
            int insercao_anterior = 0;
            int bloco_interno = 1;
            for(int insercao = 0; bloco_interno <= 25600 && blocos > 0; insercao++) {
                char c = getc(f);
                if(c == EOF) {
                    break;
                }
                if(c == '|') {
                    memoria[indice] = '\0';
                    if(memoria[0] == '0') {

                        blocos_ocupados[posicao_blocos_ocupados] = bloco_interno;
                        posicao_blocos_ocupados++;

                        if(insercao_anterior == 0) {
                            fseek(f, -21, SEEK_CUR);
                            fprintf(f, "%s," , parseiro[1]);
                        }

                        else {
                            fseek(f, insercao_anterior + 1, SEEK_SET);
                            fprintf(f, "%d",  bloco_interno);
                        }

                        fseek(f, insercao + 1, SEEK_SET);
                        if(insercao_anterior == 0)
                            insercao_anterior = insercao - 21 + strlen(parseiro[1]) + 1;
                        else
                            insercao_anterior = insercao - 21 + 1;
                        blocos--;
                        if(blocos == 0) {
                            fseek(f, -21, SEEK_CUR);
                            if (filesize <= 4096)
                                fseek(f, strlen(parseiro[1]) + 1, SEEK_CUR);
                            fprintf(f, "-1");
                            fseek(f, 21, SEEK_CUR);
                        }

                    }
                    indice = 0;
                    bloco_interno++;
                }
                else {
                    memoria[indice] = c;
                    indice++;
                }
            }
            fclose(f);
            
            f = fopen("simulacao/arquivo_simula", "r+");
            int contador_de_linha = -1;
            FILE* origem = fopen(parseiro[1], "r");
            char bloco[4097], aux;
            int posicao_bloco = 0;
            int contador_bit_map = 1;
            int indice_bit_map = 0;
            //Separação por blocos do conteudo do arquivo que está sendo passado para o sistema de arquivos do trabalho.
            for(char c = getc(f); c != EOF; c = getc(f)) {
                if(c == '\n') 
                    contador_de_linha++;
                if(contador_de_linha == 0) {
                    for(; contador_bit_map < 4096; contador_bit_map++) {
                        if(indice_bit_map >= posicao_blocos_ocupados)
                            break;
                        if(contador_bit_map == blocos_ocupados[indice_bit_map]) {
                            fprintf(f, "1");
                            indice_bit_map++;
                        }
                        else
                             getc(f);
                    }
                }
                else if(contador_de_linha >= 1)
                    break;
            }

            for(char c = getc(f); c != EOF; c = getc(f)) {
                if(c == '|')
                    contador_de_linha++;
                if(contador_de_linha == blocos_ocupados[0]) {
                    fseek(f, 1, SEEK_CUR);
                    break;
                }
            }
            int tamanho_file = filesize/10;
            fprintf(f, "%s,%ld,", parseiro[1], filesize);
            int tamanho_meta = strlen(parseiro[1]) + tamanho_file;
            int block_size = 4096;
            int indice_bloco = 1;

            while((aux = getc(origem)) != EOF) {
                if(tamanho_meta > 0) {
                    block_size = block_size - tamanho_meta;
                    tamanho_meta = 0;
                }
                else
                    block_size = 4096;
                if (posicao_bloco >= block_size) {
                    bloco[posicao_bloco] = '\0';
                    fprintf(f, "%s|", bloco);
                    fseek(f, -2 , SEEK_CUR);
                    if(indice_bloco < posicao_blocos_ocupados) {
                        for(char c = getc(f); c != EOF; c = getc(f)) {
                            if(c == '|')
                                contador_de_linha++;
                            if(contador_de_linha == blocos_ocupados[indice_bloco]){
                                indice_bloco++;
                                fseek(f, 1, SEEK_CUR);
                                break;
                            }
                        }
                    }
                    posicao_bloco = 0;
                }
                bloco[posicao_bloco] = aux;
                posicao_bloco++;
                
            }
            if(posicao_bloco != 0) {
                bloco[posicao_bloco] = '\0';
                fprintf(f, "%s|", bloco);
            }

            fclose(origem);
            fclose(f);
        }

        else if (strcmp(parseiro[0], "mkdir") == 0) {
            
        }
        
        else if (strcmp(parseiro[0], "rmdir") == 0) {
            
        }

        else if (strcmp(parseiro[0], "cat") == 0) {
            int blocos_posicoes[25600], achou = 0, anterior;
            int indice_posicao = 0;
            FILE* f = fopen("simulacao/arquivo_simula", "r+");
            char c;
            char bloco_mem[6];
            int conta_blocos = 1;
            while((c = getc(f)) != '\n') {
                if(c == '|')
                    conta_blocos++;
                if(c == parseiro[1][0]) {
                    for(int i = 1; i < strlen(parseiro[1]); i++) {
                        if(parseiro[1][i] != getc(f)){
                            break;
                        } 
                        if (i == strlen(parseiro[1]) - 1) {
                            blocos_posicoes[indice_posicao] = conta_blocos;
                            indice_posicao++;
                            getc(f);
                            int j = 0;
                            while((c = getc(f)) != ' ' && c != '|') {
                                bloco_mem[j] = c;
                                j++;
                            }
                            bloco_mem[j] = '\0';
                            if (strcmp(bloco_mem, "-1") == 0)
                                blocos_posicoes[indice_posicao] = -1;
                            else
                                blocos_posicoes[indice_posicao] = atoi(bloco_mem);
                            indice_posicao++;
                            achou = 1;
                        }
                    }
                }
                
                if(achou && blocos_posicoes[1] != -1) {

                    anterior = blocos_posicoes[0];
                    fseek(f, 21*(blocos_posicoes[0]), SEEK_SET);

                    int j = 0;
                    while((c = getc(f)) != ' ' && c != '|') {
                        bloco_mem[j] = c;
                        j++;
                    }
                    bloco_mem[j] = '\0';
                    //lemrbar que o bloco posicao ja ta erado aqui, se pa o blocomem ta dando merda. checar dps.
                    if (strcmp(bloco_mem, "-1") == 0)
                        blocos_posicoes[indice_posicao] = -1;
                    else
                        blocos_posicoes[indice_posicao] = atoi(bloco_mem);
                    indice_posicao++;
                    while(getc(f) != '|');

                    while(blocos_posicoes[indice_posicao-1] != -1) {
                        //printf("passei no while \n");
                        fseek(f, 21*(blocos_posicoes[indice_posicao-1] - anterior), SEEK_CUR);
                        anterior = blocos_posicoes[indice_posicao-1];
                        int j = 0;
                        while((c = getc(f)) != ' ' && c != '|') {
                            bloco_mem[j] = c;
                            j++;
                        }
                        bloco_mem[j] = '\0';
                        if (strcmp(bloco_mem, "-1") == 0)
                            blocos_posicoes[indice_posicao] = -1;
                        else
                            blocos_posicoes[indice_posicao] = atoi(bloco_mem);
                        indice_posicao++;
                    }
                    break;
                }
                
                else if (achou) 
                    break;
            }

            while(getc(f) != '\n');
            while(getc(f) != '\n');
            
            char aux;
            int contador_linha = 1;
            int contador_virgula = 0;
            for(int i = 0; i < indice_posicao - 1; i++) {
                while(contador_linha != blocos_posicoes[i]) {
                    while(getc(f) != '|');
                    contador_linha++;
                }
                while((aux = getc(f)) != '|') {
                    if (contador_virgula >= 2) //Mudar posteriormente para a quantidade de vírgulas referente aos metadados
                        printf("%c", aux);
                    if (aux == ',')
                        contador_virgula++;
                }
                contador_linha++;
            }

            fclose(f);
        }

        else if (strcmp(parseiro[0], "touch") == 0) {
            FILE* f = fopen("simulacao/arquivo_simula", "r+");
            
            while(getc(f) != '\n');
            
            int contador_blocos = 1;
            int contador_blocos2;
            while(getc(f) != '0')
                contador_blocos++;
            fseek(f, -1, SEEK_CUR);
            fprintf(f, "1");

            contador_blocos2 = contador_blocos;

            while(getc(f) != '\n');
            
            while(contador_blocos > 1) {
                while(getc(f) != '|');
                contador_blocos--;
            }

            fprintf(f, "%s,0,|", parseiro[1]);
            
            fseek(f, 21*(contador_blocos2-1), SEEK_SET);
            fprintf(f, "%s,-1", parseiro[1]);

            fclose(f);

        }

        else if (strcmp(parseiro[0], "rm") == 0) {
            //fudeu vamo ter q aapaagr essa merda toda agr que desgraça puta merda.
            //vcau ter q apagar tudoe e screver denovo só pra tirar um bagulhets.

        }

        else if (strcmp(parseiro[0], "ls") == 0) {
            
        }

        else if (strcmp(parseiro[0], "find") == 0) {
            
        }

        else if (strcmp(parseiro[0], "df") == 0) {
            
        }

        else if (strcmp(parseiro[0], "umount") == 0) {
            printf("%s\n", sistema_de_arquivos);
            if(remove(sistema_de_arquivos) == 0)
                printf("sistema de arquivos deletado.\n");
            else
                printf("erro na delecao do sistema de arquivos.\n");
        }

        else if (strcmp(parseiro[0], "sai") == 0) {
            exit(0);
        }

    }
}