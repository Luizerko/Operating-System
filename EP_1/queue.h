#include "processo.h"

#define bool int

void queueInit();

void queueInsert(Processo*);

Processo* queueRemove();

Processo* queueTop();

Processo* queueItera(long int);

bool queueEmpty();

long long int queueSize();

void queueFree();