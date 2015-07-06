// evolution-test program
#include "evolution.hh"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

const double PI = 3.14159265358979323846;
const double PI2 = 2.0 * PI;
const double PI3 = 7.0 * PI;

double f(double v, double w, double x, double y, double z) {
  return (5.0-v*v-w*w-x*x-y*y-z*z)/5.0;
}

double g(double v, double w, double x, double y, double z) {
  return (5.0-v*v-w*w-x*x-y*y-z*z)/5.0
    * (cos(v*17.0)+1.0)/2.0 * (cos(w*12.0)+1.0)/2.0 * (cos(x*23.0)+1.0)/2.0 
      * (cos(y*6.0)+1.0)/2.0 * (cos(z*15.0)+1.0)/2.0;
}

bool single_selection(double *h, int N, int *ind, int M);
bool multiple_selection(double *h, int N, int *ind, int M);
bool ranking(double *h, int N, int *ind);

void index_sort(int*, int);

main() {
  double T[] = {0.0, 1.0, 2.0};
  double PC; // simple crossover 
  double PM; // uniform mutation
  int M[] = {2, 4, 10, 20, 100};
  int it, im, i1, i2;
  int x;
  FILE *fp[4];
  fp[0] = fopen("ga1","w");
  fp[1] = fopen("ga2","w");
  fp[2] = fopen("ga3","w");
  fp[3] = fopen("ga4","w");
  for (x=0; x<4; ++x) {
    for (it=0; it<3; ++it) {
      for (i1=0; i1<=10; ++i1) {
	PC = double(i1)/10.0;
	for (i2=0; i2<=10; ++i2) {
	  PM = double(i2)/10.0;
	  for (im=0; im<5; ++im) {
	    double lower_bound[] = {-1.0,-1.0,-1.0,-1.0,-1.0};
	    double upper_bound[] = {+1.0,+1.0,+1.0,+1.0,+1.0};
	    Chromosome muster(5, lower_bound, upper_bound);  
	    Chromosome s[M[im]];
	    int i;
	    for (i=0; i<M[im]; ++i) {
	      s[i] = muster;
	      s[i].env.y = new double;
	      s[i].env.N = 1;
	      s[i].env[0] = 0.0; // double(i)/double(mu);
	    } 
	    Genotype::Parameters Sparam = 
	      //  a, b, prop, psc, pac, pum, pbm, pnm, maxepoch;
	    { 1.0, 1.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 10000.0 };
	    Sparam.a = T[it];
	    Sparam.psc  = PC;
	    Sparam.pum  = PM; 
	    Genotype* pS;
	    if (x%2==0) pS = new Genotype(s, M[im], Sparam);
	    else pS = new GenotypePlus(s, M[im], Sparam);
	    Genotype &S = *pS;
	    int invalid = 0;
	    int e95 = -1, m95 = -1;
	    int e = 200;
	    S.epoche = 0;
	    while(e>0) {
	      S.reproduction();
	      double val;
	      // evaluate fitnes of new generation
	      for (i=0; i<S.mu; ++i) {
		if (!S[i].valid()) {
		  S[i].fitness(0.0);
		  invalid++;
		}
		else {
		  if (x<2) val = f(S[i][0],S[i][1],S[i][2],S[i][3],S[i][4]); 
		  else val = g(S[i][0], S[i][1], S[i][2], S[i][3], S[i][4]); 
		  S[i].fitness(val);
		}
	      }
	      S.survival();
	      e--;
	      double ff = S.best().fitness();
	      double fitsum = s[0].fitness();
	      // for (i=1; i<S.mu; ++i) fitsum += s[i].fitness();
	      // cout << S.best().fitness() << '\t' << fitsum/mu << endl;
	      if (ff>=0.95 && e95<0) {
		e95 = S.epoche;
		m95 = S.epoche*M[im]-invalid;
		e = -1;
	      } 
	    } // while
	    
	    fprintf(fp[x],"%5d & %lf & %5.0lf & %5.1lf & %5.1lf  & %d \\\\\n",
		    m95, S.best().fitness(),  T[it], PC, PM, M[im]);
	  }
	}
      }
    }
  }
}
