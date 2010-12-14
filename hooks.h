#ifndef HOOKS_H
#define HOOKS_H

#include "hiredis/hiredis.h"

extern redisReplyObjectFunctions redisExtReplyObjectFunctions;

void *
createStringObject(const redisReadTask *task, char *str, size_t len);

void *
createArrayObject(const redisReadTask *task, int elements);

void *
createIntegerObject(const redisReadTask *task, long long value);

void *
createNilObject(const redisReadTask *task);

void
freeObject(void *ptr);

#endif

