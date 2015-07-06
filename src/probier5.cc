#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */
#include <iostream.h>
#include <math.h>

main () {
  double  x = 3.1415;
  double  y = 23.9;
  y = sqrt(-3.0);
  cout << "x=" << x << " y=" << y << endl;
  if (x!=x) cout << "x is not a number\n";
  if (y!=y) cout << "y is not a number\n";
  char text[] = "0.7";
  sscanf(text, "%lf", &x);
  cout << "x=" << x << " y=" << y << endl;
  printf("_%15le_%15.4lf_%15lf_\n", x, x, x);
  printf("_%13.6le_%13.6le_%13.6le_\n", -123123123.0*x, x/123123123.0, x*3.0);
  
}


