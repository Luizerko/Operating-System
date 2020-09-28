#ifndef HEADER_processo

#define HEADER_processo

typedef struct processo {
    char nome[30];
    int t0; 
    int dt; 
    int deadline;
    long int id;
} Processo;

#endif