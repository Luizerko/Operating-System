#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>


#define MAX_CARACTER 80
#define MAX_PALAVRAS 10
#define MAX_TAMANHO_PALAVRA 20

char sistema_de_arquivos[500];

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

char* pega_tempo() {
    char* tempo = malloc(200*sizeof(char));
    int indice_tempo = 0;
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    char* aux_string = asctime(timeinfo);

    for(int i = 4; i < strlen(aux_string)-1; i++) {
        if(i == 4) {
            tempo[indice_tempo] = aux_string[i] + 32;
            indice_tempo++;
        }
        else {
            if(aux_string[i] == ' ') {
                if(aux_string[i-1] != ' ') {
                    tempo[indice_tempo] = '-';
                    indice_tempo++;
                }
            }
            else {
                tempo[indice_tempo] = aux_string[i];
                indice_tempo++;
            }
        }
    }
    tempo[indice_tempo] = '\0';
    
    if(tempo[5] == '-') {
        for(int i = strlen(tempo) - 1; i >= 4; i--) {
            tempo[i+1] = tempo[i];
            if(i == 4)
                tempo[i] = '0';
        }
        tempo[indice_tempo+1] = '\0';
    }
    
    return tempo;
}


void mk_dir(char* parseiro) {
    FILE* f = fopen(sistema_de_arquivos, "r+");

    char parseiro_auxiliar[200];
    int parseiro_aux = 0;
    int barra_frente = 0;
    for(int i = 0; i < strlen(parseiro); i++) {
        if(parseiro[i] == '/') {
            barra_frente = i;
        }
    }
    if(barra_frente == 0) {
        parseiro_auxiliar[0] ='/';
        parseiro_auxiliar[1] = '\0';
        parseiro_aux = 1;
    }
    else {
        for(int i = 0; i < barra_frente; i++) {
            parseiro_auxiliar[parseiro_aux] = parseiro[i];
            parseiro_aux++;
        }
        parseiro_auxiliar[parseiro_aux] = '/';
        parseiro_aux++;
        parseiro_auxiliar[parseiro_aux] = '\0';
    }

    printf("%s\n", parseiro_auxiliar);
    
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
    char* tempo = pega_tempo();
    if(strcmp(parseiro, "/") == 0) {
        fprintf(f, "@%s,4096     ,%s,%s,%s.", parseiro, tempo, tempo, tempo);  
          
        for(int j = 0; j < 4096 - 15 - strlen(parseiro) - (3*strlen(tempo)); j++) {
            fprintf(f, " ");
        }
    }
    else {
        fprintf(f, "@%s/,4096     ,%s,%s,%s.", parseiro, tempo, tempo, tempo);

        for(int j = 0; j < 4096 - 16 - strlen(parseiro) - (3*strlen(tempo)); j++) {
            fprintf(f, " ");
        }
    }

    fprintf(f, "|");
    
    fseek(f, 21*(contador_blocos2-1), SEEK_SET);
    if(strcmp(parseiro, "/") == 0)
        fprintf(f, "@%s,-1", parseiro);
    else
        fprintf(f, "@%s/,-1", parseiro);
    
    
    if(strlen(parseiro) > 1) {
        fseek(f, 0, SEEK_SET);
        char aux;
        char aux2;
        int conta_blocos = 1;
        while((aux = getc(f)) != '\n') {
            if(aux == '|')
                conta_blocos++;
            if (aux == '@') {
                int k = 0;
                
                while((aux2 = getc(f)) != ',') {
                    
                    if (aux2 != parseiro_auxiliar[k]) {
                        break;
                    }
                    k++;
                }
                if (aux2 == ',' && k == strlen(parseiro_auxiliar)) {
                    break;
                }
            }
        }
        while(getc(f) != '\n');
        while(getc(f) != '\n');
        
        int conta_linhas = 1;
        while(conta_linhas != conta_blocos) {
            if(getc(f) == '|')
                conta_linhas++;
        }

        while(getc(f) != '.');
        while((aux = getc(f)) == '@' || aux == '/')
            while(getc(f) != '.');
        fseek(f, -1, SEEK_CUR);

        int aux3;
        for(int i = 0; i < strlen(parseiro); i++) {
            if(parseiro[i] == '/')
                aux3 = i;
        }

        parseiro_aux = 0;
        for(int i = aux3; i < strlen(parseiro); i++) {
            parseiro_auxiliar[parseiro_aux] = parseiro[i];
            parseiro_aux++;
        }
        parseiro_auxiliar[parseiro_aux] = '\0';

        fprintf(f, "@%s/,4096     ,%s,%s,%s.", parseiro_auxiliar, tempo, tempo, tempo);
    }

    fclose(f);
}

void rm(char* parseiro) {
    int blocos_posicoes[25600], achou = 0;
    int indice_posicao = 0;
    FILE* f = fopen(sistema_de_arquivos, "r+");
    char c;
    char bloco_mem[6];
    int conta_blocos = 1;
    while((c = getc(f)) != '\n') {
        if(c == '|')
            conta_blocos++;
        if(c == parseiro[0]) {
            for(int i = 1; i < strlen(parseiro); i++) {
                if(parseiro[i] != getc(f)){
                    break;
                } 
                if (i == strlen(parseiro) - 1 && getc(f) == ',') { 
                    fseek(f, -1, SEEK_CUR);
                    blocos_posicoes[indice_posicao] = conta_blocos;
                    indice_posicao++;
                    getc(f);
                    int j = 0;
                    while((c = getc(f)) != ' ' && c != '|') {
                        bloco_mem[j] = c;
                        j++;
                    }
                    bloco_mem[j] = '\0';
                    fseek(f, -j - 2 - strlen(parseiro), SEEK_CUR);
                    fprintf(f, "0                   ");
                    //printf("cheguei a achar e printei la\n");
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

            fseek(f, 21*(blocos_posicoes[0]), SEEK_SET);

            int j = 0;
            while((c = getc(f)) != ' ' && c != '|') {
                bloco_mem[j] = c;
                j++;
            }
            bloco_mem[j] = '\0';
            fseek(f, -j - 1, SEEK_CUR);
            fprintf(f, "0                   ");
            //lemrbar que o bloco posicao ja ta erado aqui, se pa o blocomem ta dando merda. checar dps.
            if (strcmp(bloco_mem, "-1") == 0)
                blocos_posicoes[indice_posicao] = -1;
            else
                blocos_posicoes[indice_posicao] = atoi(bloco_mem);
            indice_posicao++;
            while(getc(f) != '|');

            while(blocos_posicoes[indice_posicao-1] != -1) {
                fseek(f, 21*blocos_posicoes[indice_posicao-1], SEEK_SET);
                int j = 0;
                while((c = getc(f)) != ' ' && c != '|') {
                    bloco_mem[j] = c;
                    j++;
                }
                bloco_mem[j] = '\0';
                fseek(f, -j - 1, SEEK_CUR);
                fprintf(f, "0                   ");
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

    int indice_aux = 0;
    int conta_bitmap = 1;
    while(getc(f) != '\n') {
        if(indice_aux < indice_posicao && blocos_posicoes[indice_aux] == conta_bitmap) {
            fseek(f, -1, SEEK_CUR);
            fprintf(f, "0");
            indice_aux++;
        }
        conta_bitmap++;
    }
    
    int contador_linha = 1;
    for(int i = 0; i < indice_posicao - 1; i++) {
        while(contador_linha != blocos_posicoes[i]) {
            while(getc(f) != '|');
            contador_linha++;
        }
        fprintf(f, "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ");
        contador_linha++;
    }
    
    char dir[200];
    char nome_remocao[200];
    int indice_nome_remocao = 0;
    int indice_dir = 0;
    int indice_pasta;
    
    if(parseiro[0] != '@') {
        for(int i = 0; i < strlen(parseiro); i++) {
            if(parseiro[i] == '/') {
                indice_pasta = i;
                indice_nome_remocao = 0;
            }
            nome_remocao[indice_nome_remocao] = parseiro[i];
            indice_nome_remocao++;
        }

        nome_remocao[indice_nome_remocao] = '\0';
        
        if(indice_pasta == 0) {
            dir[0] = '/';
            dir[1] = '\0';
            indice_dir = 1;
        }
        else {
            for(int i = 0; i < indice_pasta; i++) {
                if(parseiro[i] == '/')
                    indice_dir = 0;
                dir[indice_dir] = parseiro[i];
                indice_dir++;
            }
            dir[indice_dir] = '/';
            indice_dir++;
            dir[indice_dir] = '\0';
        }
    }
    
    else {
        for(int i = 0; i < strlen(parseiro)-1; i++) {
            if(parseiro[i] == '/') {
                indice_pasta = i;
                indice_nome_remocao = 0;
            }
            nome_remocao[indice_nome_remocao] = parseiro[i];
            indice_nome_remocao++;
        }
        nome_remocao[indice_nome_remocao] = '/';
        indice_nome_remocao++;
        nome_remocao[indice_nome_remocao] = '\0';

        if(indice_pasta == 1) {
            dir[0] = '/';
            dir[1] = '\0';
            indice_dir = 1;
        }

        else {
            for(int i = 1; i < indice_pasta; i++) {
                /*if(parseiro[i] == '/')
                    indice_dir = 0;*/
                dir[indice_dir] = parseiro[i];
                indice_dir++;
            }
            dir[indice_dir] = '/';
            indice_dir++;
            dir[indice_dir] = '\0';
        }
    }

    fseek(f, 0, SEEK_SET);

    char aux;
    char aux2;
    conta_blocos = 1;
    while((aux = getc(f)) != '\n') {
        if(aux == '|')
            conta_blocos++;
        if (aux == '@') {
            int k = 0;
            
            while((aux2 = getc(f)) != ',') {
                
                if (aux2 != dir[k]) {
                    break;
                }
                k++;
            }
            if (aux2 == ',' && k == strlen(dir)) {
                break;
            }
        }
    }

    while(getc(f) != '\n');
    while(getc(f) != '\n');

    char auxiliar_palavra_remocao[200];
    char aux3[4096];
    int indice_aux3 = 0;
    int indice_auxiliar_palavra_remocao = 0;
    int aux4 = 1;
    char string[200];
    if(parseiro[0] == '@') {
        string[0] = '@';
        string[1] = '\0';
        strcat(string, nome_remocao);
        for(int i = 0; i < strlen(string); i++)
            nome_remocao[i] = string[i];
        nome_remocao[strlen(string)] = '\0';
    }

    while(getc(f) != '.');
    while(1) {
        indice_auxiliar_palavra_remocao = 0;
        aux4 = 1;
        //while(getc(f) != '.');
        
        while((aux2 = getc(f)) != ',') {
            auxiliar_palavra_remocao[indice_auxiliar_palavra_remocao] = aux2;
            indice_auxiliar_palavra_remocao++;
        }
        auxiliar_palavra_remocao[indice_auxiliar_palavra_remocao] = '\0';

        while(getc(f) != '.')
            aux4++;
        aux4++;
        
        // /lalala,123./teste,21332./lelele,240./tes2,232.

        if(strcmp(auxiliar_palavra_remocao, nome_remocao) == 0) {//aqaui???
            while((aux2 = getc(f)) != ' ') {
                fseek(f, -1, SEEK_CUR);
                while((aux2 = getc(f)) != '.' ) {
                    aux3[indice_aux3] = aux2;
                    indice_aux3++;
                }
                aux3[indice_aux3] = aux2;
                indice_aux3++;
            }
            aux3[indice_aux3] = '\0';
            fseek(f, -(indice_aux3 + indice_auxiliar_palavra_remocao + aux4 + 1), SEEK_CUR);
            fprintf(f, "%s", aux3);
            if(parseiro[0] != '@') {
                while((aux2 = getc(f)) != '|') {
                    fseek(f, -1, SEEK_CUR);
                    fprintf(f, " ");
                }
                break;
            }
            else {
                while((aux2 = getc(f)) != '|') {
                    fseek(f, -1, SEEK_CUR);
                    fprintf(f, " ");
                }
                break;
            }
        }

    }

    fclose(f);
}

void ls(char* parseiro) {
    FILE* f = fopen(sistema_de_arquivos, "r+");
    char aux;
    int conta_blocos;
    if(strlen(parseiro) != 1) {
        fseek(f, 0, SEEK_SET);
        char aux2;
        conta_blocos = 1;
        while((aux = getc(f)) != '\n') {
            if(aux == '|')
                conta_blocos++;
            if (aux == '@') {
                int k = 0;
                
                while((aux2 = getc(f)) != ',') {
                    
                    if (aux2 != parseiro[k]) {
                        break;
                    }
                    k++;
                }
                //mudar mudar mudar STOP THE COUNT.
                if (aux2 == ','  && k == strlen(parseiro)) {
                    break;
                }
            }
        }

        while(getc(f) != '\n');
        while(getc(f) != '\n');

        char palavra[200];
        int ind_palavra = 0;
        while(conta_blocos > 1) {
            while(getc(f) != '|');
            conta_blocos--;
        }
        
        while(getc(f) != '.');
        while(1) {

            while((aux = getc(f)) != '.') {
                palavra[ind_palavra] = aux;
                ind_palavra++;
                if (aux == ' ' || aux == '|')
                    break;
            }

            palavra[ind_palavra] = '\0';
            ind_palavra = 0;
            if(palavra[0] == '@') {
                palavra[0] = ' ';
                printf("(Diretorio)");
            }
            printf("%s \n", palavra);

            if(aux == '.') {
                if(getc(f) == ' ')
                    break;
                fseek(f, -1, SEEK_CUR);
            }
            
            else
                while(getc(f) != '.');

            if((aux = getc(f)) == EOF || aux == ' ' || aux == '.')
                break;
            fseek(f, -1, SEEK_CUR);

        }
    }

    else {
        while((aux = getc(f)) != '\n');
        while((aux = getc(f)) != '\n');

        while((aux = getc(f)) != EOF && aux != '.' && aux != '|');
        while((aux = getc(f)) != EOF && aux != '|') {
            fseek(f, -1, SEEK_CUR);
            if(aux == EOF || aux == '|')
                break;
            char palavra[200];
            int indice_palavra = 0;
            while((aux = getc(f)) != EOF && aux != ' ' && aux != '|' && aux != '.') {
                palavra[indice_palavra] = aux;
                indice_palavra++;
                
            }
            if(aux == EOF || aux == '|')
                break;
            
            palavra[indice_palavra] = '\0';
            indice_palavra++;
            
            if(indice_palavra > 1   ) {
                if (palavra[0] == '@') {
                    palavra[0] = ' ';
                    printf("(Diretorio)%s\n", palavra);
                }
                else
                    printf("%s\n", palavra);
            }
            indice_palavra = 0;
        }
        
    }
    
    fclose(f);
}

void find_me(char* diretorio, char* arquivo) {
    FILE* f = fopen(sistema_de_arquivos, "r+");

    char aux;
    char path_auxiliar[200], arquivo_auxiliar[200];
    int indice_path = 0, indice_arquivo = -1;

    while((aux = getc(f)) != '\n') {
        if(aux == '/') {
            indice_path = 0;
            fseek(f, -1, SEEK_CUR);
            while((aux = getc(f)) != ',') {
                path_auxiliar[indice_path] = aux;
                indice_path++;
            }
            path_auxiliar[indice_path] = '\0';
            if(strstr(path_auxiliar, diretorio) != NULL) {
                for(int i= 0; i < strlen(path_auxiliar); i++) {
                    if(path_auxiliar[i] == '/')
                        indice_arquivo = 0;
                    arquivo_auxiliar[indice_arquivo] = path_auxiliar[i];
                    indice_arquivo++;
                }
                arquivo_auxiliar[indice_arquivo] = '\0';

                if(strcmp(arquivo_auxiliar, arquivo) == 0)
                    printf("%s\n", path_auxiliar);
            }
        }
    }

    fclose(f);
    
}

void rm_dir(char* parseiro) {
    FILE* f = fopen(sistema_de_arquivos, "r+");
    
    char aux, aux2;
    int conta_blocos = 1;

    while((aux = getc(f)) != '\n') {
        if(aux == '|')
            conta_blocos++;
        if (aux == '@') {
            int k = 0;
            char problemao[200];
            int indice_problemao = 0;
            
            while((aux2 = getc(f)) != ',') {
                problemao[indice_problemao] = aux2;
                indice_problemao++;

                if (aux2 != parseiro[k]) {
                    break;
                }
                k++;
            }
            problemao[indice_problemao] = '\0';
            if (aux2 == ',' && strcmp(problemao, parseiro) == 0) {
                break;
            }
        }
    }

    while(getc(f) != '\n');
    while(getc(f) != '\n');
    
    while(conta_blocos > 1) {
        while(getc(f) != '|');
        conta_blocos--;
    }

    while(1) {  
        while(getc(f) != '.');
        if((aux = getc(f)) == ' ' || aux == EOF || aux == '|')
            break;
        fseek(f, -1, SEEK_CUR);
        
        char apagando[200];
        int indice_apagando = 0;
        while((aux = getc(f)) != ',') {
            apagando[indice_apagando] = aux;
            indice_apagando++;
        }
        apagando[indice_apagando] = '\0';
        if(apagando[0] == '@') {
            int i = 0;
            while(i < indice_apagando) {
                apagando[i] = apagando[i+1];
                i++;
            }
            apagando[i] = '\0';            //se pa n faz nd e funciona.
            /*int i = indice_apagando;
            while(i >= 0) {
                apagando[i+1] = apagando[i];
                i--;
            }
            apagando[indice_apagando+1] = '\0';
            apagando[0] = '/';*/
            //fclose(f);
            char string3[200];
            string3[0] = '@';
            strcpy(string3, parseiro);
            string3[strlen(parseiro) - 1] = '\0';
            strcat(string3, apagando);
            printf("apaGAnDO: %s\n", string3);
            rm_dir(string3);
            printf("apagay: %s\n", string3);

        }
        else {
            char string2[200];
            strcpy(string2, parseiro);
            string2[strlen(parseiro) - 1] = '\0';
            strcat(string2, apagando);
            printf("apagando: %s\n", string2);
            rm(string2);
            printf("apagay: %s\n", string2);
        }
    }
    
    char string[200];
    string[0] = '@';
    string[1] = '\0';
    strcat(string, parseiro);

    printf("apagando: %s\n", string);
    rm(string);
    printf("apagay: %s\n", string);
    fclose(f);
}

int main() {
    strcpy(sistema_de_arquivos, "/home/lui/Documents/IME/2020.2/SO/SO_rep/EP_3/simulacao/arquivo_simula");
    while(1) {
        printf("\n[ep3]: ");
        char* linha = readline("");
        printf("\n");
        char** parseiro = parser(linha);

        if (strcmp(parseiro[0], "mount") == 0) {
            for(int i = 0; i < strlen(parseiro[1]); i++) {
                sistema_de_arquivos[i] = parseiro[1][i];
            }
            sistema_de_arquivos[strlen(parseiro[1])] = '\0';
            if(access(sistema_de_arquivos, F_OK) == -1) {
                FILE* f = fopen(sistema_de_arquivos, "a+"); //lembrar de trocar por parseiro[1].
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
                mk_dir("/");
            }
        }

        //cp "path_do_arquivo" "path_da_pasta"
        //cp /teste /lalala/ (copia o arquivo "teste" que está em "/" para "/lalala/")
        else if (strcmp(parseiro[0], "cp") == 0) {
            
            FILE* f = fopen(sistema_de_arquivos, "a+");

            struct stat finfo;
            fstat(open(parseiro[1], O_RDONLY), &finfo);
            off_t filesize = finfo.st_size;
            fclose(f);
            
            char* tempo = pega_tempo();
            
            int posicao_blocos_ocupados = 0;
            int blocos_ocupados[25600];

            long int blocos = (long int)ceil((float)filesize/(float)4096);
            //printf("%ld\n", blocos);

            f = fopen(sistema_de_arquivos, "r+");
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
                            if (strlen(parseiro[2]) == 1) // Mudei aquiiii
                                fprintf(f, "%s," , parseiro[1]);
                            else {
                                fprintf(f, "%s" , parseiro[2]);
                                fseek(f, -1, SEEK_CUR);
                                fprintf(f, "%s," , parseiro[1]);
                            }
                        }

                        else {
                            fseek(f, insercao_anterior + 1, SEEK_SET);
                            fprintf(f, "%d",  bloco_interno);
                        }

                        fseek(f, insercao + 1, SEEK_SET);
                        if(insercao_anterior == 0)
                            insercao_anterior = insercao - 21 + strlen(parseiro[1]) + strlen(parseiro[2]) - 1 + 1;
                        else
                            insercao_anterior = insercao - 21 + 1;
                        blocos--;
                        if(blocos == 0) {
                            fseek(f, -21, SEEK_CUR);
                            if (filesize <= 4096)
                                fseek(f, strlen(parseiro[1]) + strlen(parseiro[2]) - 1 + 1, SEEK_CUR);
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
            
            f = fopen(sistema_de_arquivos, "r+");
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

            char parseiro_auxiliar[200];
            strcpy(parseiro_auxiliar, parseiro[2]);
            int ind = 0;
            char aux_ind[50];
            for(int i = 0; i < strlen(parseiro[1]); i++) {
                aux_ind[ind] = parseiro[1][i];
                ind++;
                if(parseiro[1][i] == '/') {
                    ind = 0;
                }
            }
            aux_ind[ind] = '\0';
            ind++;
            strcat(parseiro_auxiliar, aux_ind);

            int tamanho_file = filesize/10;
            fprintf(f, "%s,%ld,%s,%s,%s", parseiro_auxiliar, filesize, tempo, tempo, tempo);
            int tamanho_meta = strlen(parseiro_auxiliar) + tamanho_file;
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
                fprintf(f, "%s", bloco);
                for(int j = 0; j < 4096 - strlen(bloco); j++) {
                    fprintf(f, " ");
                }
                fprintf(f, "|");
            }

            fseek(f, 0, SEEK_SET);
            char aux2;
            int conta_blocos = 1;
            while((aux = getc(f)) != '\n') {
                if(aux == '|')
                    conta_blocos++;
                if (aux == '@') {
                    int k = 0;
                    char problemao[200];
                    int indice_problemao = 0;
                    
                    while((aux2 = getc(f)) != ',') {
                        problemao[indice_problemao] = aux2;
                        indice_problemao++;

                        if (aux2 != parseiro[2][k]) {
                            break;
                        }
                        k++;
                    }
                    problemao[indice_problemao] = '\0';
                    if (aux2 == ',' && strcmp(problemao, parseiro[2]) == 0) {
                        break;
                    }
                }
            }
            if(aux == '\n') {
                printf("Diretório inexistente!\n");
            }

            while(getc(f) != '\n');
            while(getc(f) != '\n');
            
            while (conta_blocos > 1) {
                while (getc(f) != '|');
                conta_blocos--;
            }
            
            /*
            while((aux = getc(f)) != ',');
            char numero_string[10];
            int indice_numero_string = 0;
            long numero;
            while((aux = getc(f)) != ' ' && aux != '.') {
                numero_string[indice_numero_string] = aux;
                indice_numero_string++;
            }
            numero_string[indice_numero_string] = '\0';
            numero = atoi(numero_string);
            numero += filesize;
            fseek(f, -(indice_numero_string + 1), SEEK_CUR);
            fprintf(f, "%ld", numero);
            */

            while(getc(f) != '.');
            while((aux = getc(f)) == '@' || aux == '/')
                while(getc(f) != '.');
            fseek(f, -1, SEEK_CUR);
            fprintf(f, "%s,%ld,%s,%s,%s.", parseiro[1], filesize, tempo, tempo, tempo);

            fclose(origem);
            fclose(f);
        }

        //mkdir "path_da_pasta_nova"
        //mkdir /lalala (cria uma pasta nova "lalala" no path raiz "/")
        else if (strcmp(parseiro[0], "mkdir") == 0) {
            mk_dir(parseiro[1]);
        }
        
        //rmdir "path_da_pasta"
        //rmdir /lalala/ (remove a pasta "lalala/" e tudo que está dentro dela do path raiz "/")
        else if (strcmp(parseiro[0], "rmdir") == 0) {
            rm_dir(parseiro[1]);
        }

        //cat "path_do_arquivo"
        //cat /lalala/teste (lê o arquivo "teste" que está no path raiz "/lalala/")
        else if (strcmp(parseiro[0], "cat") == 0) {
            int blocos_posicoes[25600], achou = 0;
            int indice_posicao = 0;
            FILE* f = fopen(sistema_de_arquivos, "r+");
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
                        if (i == strlen(parseiro[1]) - 1 && getc(f) == ',') { // e se o nome for igual e tiver coisa dps? tipo teste1 e teste11.
                            fseek(f, -1, SEEK_CUR);
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
                        fseek(f, 21*blocos_posicoes[indice_posicao-1], SEEK_SET);
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
            int tamanho_arquivo = -1;
            char tamanho_char[6];
            int aux_char = 0;
            int flag_tamanho_arquivo = 0;
            for(int i = 0; i < indice_posicao - 1; i++) {
                while(contador_linha != blocos_posicoes[i]) {
                    while(getc(f) != '|');
                    contador_linha++;
                }
                while((aux = getc(f)) != '|') {
                    if(contador_virgula == 1) {
                        tamanho_char[aux_char] = aux;
                        aux_char++;
                    }
                    if (contador_virgula >= 2) { //Mudar posteriormente para a quantidade de vírgulas referente aos metadados
                        if(!flag_tamanho_arquivo && i == indice_posicao-2) {
                            flag_tamanho_arquivo = 1;
                            tamanho_char[aux_char] = '\0';
                            tamanho_arquivo = atoi(tamanho_char);
                            tamanho_arquivo = tamanho_arquivo % 4096;
                        }
                        if(i < indice_posicao - 2 || tamanho_arquivo > 0) {
                            printf("%c", aux);
                            tamanho_arquivo--;
                        }
                        else
                            break;
                    }
                    if (aux == ',')
                        contador_virgula++;
                }
                contador_linha++;
            }

            fclose(f);
        }

        //touch "path_do_arquivo"
        //touch /lalala/teste (cria um arquivo vazio "teste" no path raiz "/lalala/")
        else if (strcmp(parseiro[0], "touch") == 0) {
            FILE* f = fopen(sistema_de_arquivos, "r+");
            
            char arquivo[200];
            int indice_arquivo = 0;
            char aux;
            int conta_blocos = 1;
            int flag_criacao = 0;
            while((aux = getc(f)) != '\n') {
                if(aux == '|') {
                    conta_blocos++;
                    indice_arquivo = 0;
                    while((aux = getc(f)) != ',') {
                        arquivo[indice_arquivo] = aux;
                        indice_arquivo++;
                    }
                    arquivo[indice_arquivo] = '\0';
                    if(strcmp(arquivo, parseiro[1]) == 0) {
                        flag_criacao = 1;
                        break;
                    }
                }
            }
            if(aux != '\n')
                while((aux = getc(f)) != '\n');

            if(!flag_criacao) {
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
                char* tempo = pega_tempo();
                fprintf(f, "%s,0,%s,%s,%s", parseiro[1], tempo, tempo, tempo);

                for(int j = 0; j < 4096 - 3 - strlen(parseiro[1]); j++) {
                    fprintf(f, " ");
                }
                fprintf(f, "|");
                
                fseek(f, 21*(contador_blocos2-1), SEEK_SET);
                fprintf(f, "%s,-1", parseiro[1]);

                fclose(f);
            }

            else {
                while((aux = getc(f)) != '\n');
                char* tempo = pega_tempo();
                while(conta_blocos > 1) {
                    while(getc(f) != '|');
                    conta_blocos--;
                }
                while(getc(f) != ',');
                while(getc(f) != ',');
                while(getc(f) != ',');
                while(getc(f) != ',');
                fprintf(f, "%s", tempo);
                fclose(f);
            }
        }

        //rm "path_do_arquivo"
        //rm /lalala/teste (remove um arquivo ou diretorio "teste" do path raiz "/lalala/")
        else if (strcmp(parseiro[0], "rm") == 0) {
            rm(parseiro[1]);
        }

        //ls "path_da_pasta"
        //ls /lalala/ (lista todos os arquivos e diretórios que encontram-se em "/lalala/")
        else if (strcmp(parseiro[0], "ls") == 0) {
            ls(parseiro[1]);
        }

        //find "path_da_pasta" "path_do_arquivo"
        //find /lalala/ /teste (imprime o caminho completo de todos os arquivos de nome "/teste" que estão dentro da pasta "/lalala" ou dentro de qualquer pasta dentro dela")
        else if (strcmp(parseiro[0], "find") == 0) {
            find_me(parseiro[1], parseiro[2]);
        }

        //df (imprime as devidas informações sobre o sistema de arquivos)
        else if (strcmp(parseiro[0], "df") == 0) {
            FILE* f = fopen(sistema_de_arquivos, "r+");
            int contador_arquivos = 0;
            int contador_diretorios = 0;
            int inicio_bloco[25600];
            int indice_inicio_bloco = 0;
            int blocos = 1;
            char aux;
            while((aux = getc(f)) != '\n') {
                if(aux == '|')
                    blocos++;
                if(aux == '/') {
                    while(getc(f) != ',');
                    inicio_bloco[indice_inicio_bloco] = blocos;
                    indice_inicio_bloco++;
                    contador_arquivos++;
                }
                else if(aux == '@') {
                    while(getc(f) != ',');
                    contador_diretorios++;
                }
            }
            printf("Quantidade de arquivos: %d\n", contador_arquivos);
            printf("Quantidade de diretórios: %d\n", contador_diretorios);

            int contador_espacos_vazios = 0;
            while((aux = getc(f)) != '\n')
                if(aux == '0')
                    contador_espacos_vazios++;
            printf("Espaço livre: %d\n", 4096*contador_espacos_vazios);

            int contador_linha = 1;
            int blocos_verificados = 0;
            int espaco_desperdicado = 0;
            while((aux = getc(f)) != EOF) {
                if(blocos_verificados < indice_inicio_bloco && contador_linha == inicio_bloco[blocos_verificados]) {
                    int conta_nome = 1;
                    while((aux = getc(f)) != ',') {
                        conta_nome++;
                    }
                    int j = 0;
                    char bloco_mem[6];
                    while((aux = getc(f)) != ',') {
                        bloco_mem[j] = aux;
                        j++;
                    }
                    conta_nome += (j + 2);
                    bloco_mem[j] = '\0';
                    espaco_desperdicado += 4096 - (atoi(bloco_mem) + conta_nome) % 4096;
                    blocos_verificados++;
                }
                else if(aux == '@') {
                    int desperdicio_diretorio = 1;
                    while((aux = getc(f)) != '|') {
                        if(aux == '.')
                            if(getc(f) == ' ') {
                                desperdicio_diretorio++;
                                espaco_desperdicado += 4096 - desperdicio_diretorio;
                            }
                        desperdicio_diretorio++;
                    }
                }
                if(aux == '|')
                    contador_linha++;
            }
            printf("Espaço desperdiçado: %d\n", espaco_desperdicado);

            fclose(f);

        }

        //umount (desmonta o sistema de arquivos alocado em "sistema_de_arquivos")
        else if (strcmp(parseiro[0], "umount") == 0) {
            printf("%s\n", sistema_de_arquivos);
            if(remove(sistema_de_arquivos) == 0)
                printf("sistema de arquivos deletado.\n");
            else
                printf("erro na delecao do sistema de arquivos.\n");
        }

        //sai (termina o processo e sai do programa)
        else if (strcmp(parseiro[0], "sai") == 0) {
            exit(0);
        }

    }
}