// evolution-test program
#include "evolution.hh"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

const double PI = 3.14159265358979323846;
const double PI2 = 2.0 * PI;
const double PI3 = 7.0 * PI;

double f(double x, double y, double z) {
  //return (sin(x)*sin(y)*sin(z))/(1.0+x*y*z);
  double a = (3.0-x*x-y*y-z*z)/3.0;
  return a;
}

double g(double v, double w, double x, double y, double z) {
  return (5.0-v*v-w*w-x*x-y*y-z*z)/5.0
    * (cos(v*17.0)+1.0)/2.0 * (cos(w*12.0)+1.0)/2.0 * (cos(x*23.0)+1.0)/2.0 
      * (cos(y*6.0)+1.0)/2.0 * (cos(z*15.0)+1.0)/2.0;
}

double f(double v, double w, double x, double y, double z) {
  return (5.0-v*v-w*w-x*x-y*y-z*z)/5.0;
}


void sub(double T, double D, int M, int L) {
  int e = 200;
  int e50 = -1, e85 = -1, e95 = -1;
  int m50 = -1, m85 = -1, m95 = -1;
  ES S(M, L, 5);
  int invalid = 0;
  S.epoch = 0;
  while (e>0) {
    S.epoch++;
    S.reproduction(T, D);
    for (int i=0; i<S.lambda; ++i) {
      if (!S.chi[i].valid()) {
	S.chi[i].fitness = 0.0;
	invalid++;
      }
      else S.chi[i].fitness = 
	g(S.chi[i].x[0], S.chi[i].x[1], S.chi[i].x[2], 
	  S.chi[i].x[3], S.chi[i].x[4]);
      //      cout << '\t' << S.chi[i] << endl;
    }
    S.selectplus();
    e--;
    double w = S.w();
    double f = S.best().fitness;
    if (f>=0.5 && e50<0) {
      e50 = S.epoch;
      m50 = S.epoch*L-invalid;
    }
    if (f>=0.85 && e85<0) {
      e85 = S.epoch;
      m85 = S.epoch*L-invalid;
    }
    if (f>=0.95 && e95<0) {
      e95 = S.epoch;
      m95 = S.epoch*L-invalid;
	e = -1;
    }
  } // while
  // Ausgabe
  if (m95>0) {
    FILE *fp = fopen("ess","a");
    fprintf(fp,"%d & & %d %lf & %lf & %lf & %d  & %d \\\\\n",
	    m95, invalid, S.best().fitness,  T, D, M, L);
    fclose(fp);
  }
}


main() {
  int m, l, e;
  //  S.optimize(1.0 , 1.0, e, &g);
  double T[] = {1.0};
  double D[] = {0.1, 0.2, 0.3};
  int M[] = {1, 5, 10};
  int L[] = {1, 5, 10};
  int it, id, im, il;
  for (it=0; it<1; ++it) {
    for (id=0; id<3; ++id) {
      for (im=0; im<3; ++im) {
	for (il=0; il<3; ++il) {
	  sub(T[it], D[id], M[im], L[il]);
	}
      }
    }
  } 
}

