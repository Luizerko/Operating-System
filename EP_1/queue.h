#include "processo.h"

#define bool int

void queueInit();

void queueInsert(Processo*);

Processo* queueRemove();

bool queueEmpty();

int queueSize();

void queueFree();