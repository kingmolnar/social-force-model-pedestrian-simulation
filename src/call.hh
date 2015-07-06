#ifndef _CALL_HH_
#define _CALL_HH_
#include <stdio.h>
#include <iostream.h>
#include <sys/types.h>

class program {
private: 
  int fildes[2]; // 0 read, 1 write
  FILE *fp;
  char *name;
  pid_t pid;
public: 
  program(char*, char**, char**);
  ~program();
  FILE* operator FILE*() {return fp;}
};

#endif
