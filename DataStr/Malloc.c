#define RCSID "$Id: Malloc.c,v 1.3 2001-03-03 19:21:19 geuzaine Exp $"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "Malloc.h"
#include "Message.h"

void *Malloc(size_t size)
{
  void *ptr;

  if (!size) return(NULL);
  ptr = malloc(size);
  if (ptr == NULL)
    Msg(ERROR, "Out of memory (buy some more RAM!)");
  return(ptr);
}

void *Calloc(size_t num, size_t size)
{
  void *ptr;

  if (!size) return(NULL);
  ptr = calloc(num, size);
  if (ptr == NULL)
    Msg(ERROR, "Out of memory (buy some more RAM!)");
  return(ptr);
}

void *Realloc(void *ptr, size_t size)
{
  if (!size) return(NULL);
  ptr = realloc(ptr,size);
  if (ptr == NULL)
    Msg(ERROR, "Out of memory (buy some more RAM!)");
  return(ptr);
}

void Free(void *ptr)
{
  if (ptr == NULL) return;
  free(ptr);
}
