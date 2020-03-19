#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void * malloc_checked(size_t size){
  void * mem = malloc(size);
  if(mem == NULL){
    //something went wrong, likely not recoverable
    fputs("unable to allocate memory\n", stderr);
    exit(-1);
  }else{
    return mem;
  }
}
