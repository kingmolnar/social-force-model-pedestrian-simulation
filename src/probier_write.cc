#include <stdio.h>
#include <stdlib.h>
//#include <sys/uio.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */

main () {

  double a[2];
  double& x = a[0];
  double& y = a[1];

  a[0] = 1.0; a[1] = 2.0;
  cout << x << ", " << y << endl;

  char txt[] = {"Hallo Leute\nIch glaube heute wird es ganz nett.\n"
		"Werden wir genug Spass haben?\n" };
  int f = open("probe.text", O_RDWR | O_CREAT, 256+128+32+4);
  printf("FILE=%d\n", f);
  if (f>-1) write(f, txt, sizeof(txt));
}
