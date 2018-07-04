/* 
   test.h f"ur Test-Anweisungen.
   Z.B. zur Ausgabe von Zwischen-Ergebnissen:
      test(printf("x=%lf\n", x));

   Mit der Compiler-Option -DNTEST (cc -c -DNTEST foo.c) 
   verschwindet die Anweisung aus dem Programm. D.h. Es wird dann auch keine
   zus"atzliche Rechenzeit verbraucht

   (viel zu simpel, um seinen Namen darunter zu schreiben)
*/

#undef test  
#ifdef NTEST
#define test(EX)
#define announce(STR)
#else
#define test(EX) (EX)
#include <stdio.h>
#define announce(STR) printf(STR)
#endif

    


