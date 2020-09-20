#include "processo.h"

#define bool int

void queueInit();

void queueInsert(Processo*);

Processo* queueRemove();

bool queueEmpty();

long long int queueSize();

void queueFree();