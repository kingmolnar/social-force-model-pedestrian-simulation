#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

#ifndef NULL
#define NULL ((void*)0)
#endif


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>
#include <time.h>
#include <sys/timeb.h>

class TextFile {
  FILE *stream;
  bool flag;
  int li, co; 
  char *name;
  char *mod;
  char *buffer;
  const unsigned short int buffersize = 512;
  unsigned short int bufferind;
public:
  TextFile(char*, char*);
  ~TextFile();
  int line()  {return li;}
  int column() {return co;} 
  void token(char*);
  void reopen();
  bool isopen() {return flag;}
  operator bool () {return flag;}
  bool eof() {return isopen() ? feof(stream) : 1;}
  FILE* operator()() {return stream;}
  char *lastline(int n = 1);
};

#endif






