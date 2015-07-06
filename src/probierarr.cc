
#include <iostream.h>
#include "arrlst.h"
#include "vector.hh"

typedef char *name;

void main() {
  register int i;
  /*
   *	arrlst<int> intarrlst;
   *	intarrlst[500]=500;
   *	for (i=0; i<=370; i++) {intarrlst[i]=i;}
   *	for (i=0; i<=500;i++){ cout<<i<<": "<<intarrlst[i]<<endl;}
   */

  arrlst<name> namelist;
  arrlst<char*> namelist2;
  
  arrlst<Vector> pfeile;
  
  arrlst<float> A;
  class WAS {
    double a, b, c;
    int e, f, g;
  };

  arrlst<WAS*> w;
  
  namelist[0]="Joachim Keltsch";
  namelist[5]="Jens Starke";
  namelist[7]="Peter Molnar";
  namelist[110]="Polizei";
  namelist[112]="Feuerwehr";
  namelist[66]="Alexander Kaefer";
  namelist[500]="Dirk Helbing";
  
  pfeile[17] = Vector(0.0,1.0);
  pfeile[34] = Vector(1.0,1.0);
  pfeile[89] = Vector(7.0,1.0);
  pfeile[11] = Vector(3.0,1.0);
  
  for (i=0; i<=5; i++)
    {
      cout << i << ": " << namelist[i] << endl;
    }
}
