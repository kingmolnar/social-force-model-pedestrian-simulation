#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

main() {
  int i = 0;
  char c;
  while((c=getchar()) != EOF) {
    if (c == '\n' ) {
      if (i) putchar(c);
      else {
	i = 1;
	putchar('\t');
      }
    }
    else {
      putchar(c);
      i = 0;
    }
  } 
}

