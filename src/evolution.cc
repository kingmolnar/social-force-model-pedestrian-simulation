#include <iostream.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "evolution.hh"

#undef test
#define NO_OUTPUT
#ifndef NO_OUTPUT
#define test(A) A
#else 
#define test(A)
#endif

extern "C" {
 // double rint(double);
  // long random();
//  double exp(double);
}

// zuafallszahlen nach Numerical Recipies
#include <math.h>

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

static float ran1(long *idum) {
  int j;
  long k;
  static long iy=0;
  static long iv[NTAB];
  float temp;
  
  if (*idum <= 0 || !iy) {
    if (-(*idum) < 1) *idum=1;
    else *idum = -(*idum);
    for (j=NTAB+7;j>=0;j--) {
      k=(*idum)/IQ;
      *idum=IA*(*idum-k*IQ)-IR*k;
      if (*idum < 0) *idum += IM;
      if (j < NTAB) iv[j] = *idum;
    }
    iy=iv[0];
  }
  k=(*idum)/IQ;
  *idum=IA*(*idum-k*IQ)-IR*k;
  if (*idum < 0) *idum += IM;
  j=iy/NDIV;
  iy=iv[j];
  iv[j] = *idum;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;
}
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
/* (C) Copr. 1986-92 Numerical Recipes Software 5.){2ptNsz4'>4. */


static float gasdev(long *idum) {
  float ran1(long *idum);
  static int iset=0;
  static float gset;
  float fac,rsq,v1,v2;
  
  if  (iset == 0) {
    do {
      v1=2.0*ran1(idum)-1.0;
      v2=2.0*ran1(idum)-1.0;
      rsq=v1*v1+v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac=sqrt(-2.0*log(rsq)/rsq);
    gset=v1*fac;
    iset=1;
    return v2*fac;
  } else {
    iset=0;
    return gset;
  }
}
/* (C) Copr. 1986-92 Numerical Recipes Software 5.){2ptNsz4'>4. */


static double rnd(double a, double b) {
  static long idum = -1931;
  return (b - a)*ran1(&idum) + a;
}

static double rnd() {
  static long idum = -1933;
  return ran1(&idum);
}


static inline int rnd(int k, int l) {
  //  return int(rint(rnd(double(k), double(l))));
  return int(floor(rnd(double(k), double(l))+0.5));
}

static double rndg(double a, double b) {
  static long idum = -1932;
  return a + b * gasdev(&idum);
}

static int irndsel(double *h, int n) {
  if (n<=0) return -1;
  int i;
  double sum = 0.0;
  double q[n];
  for (i=0; i<n; i++) {
    sum += h[i];
    q[i] = sum;
  }
  double x = sum * rand()/2147483647.0;
  for (i=0; i<n; i++) {
    if (x<q[i]) break;
  }
  if (i>=n) i = n-1;
  h[i] = 0.0;
  return i;
}


inline int rnd(int n) { return rand()%n;}

bool single_selection(double *h, int N, int *ind, int M) {
  // all h[i] > 0 !
  // destroyes h-vector !
  if (!h || !ind || !N || !M || M>N) return 0;
  //  if (!ind) ind = new int[M];
  int i;
  double sum;
  while (M>0) {
    sum = h[0];
    for (i=1; i<N; i++) sum += h[i];
    double x = rnd(0.0, sum);
    for (i=0; i<N; i++) {
      if (x<h[i]) break;
      x -= h[i];
    }
    if (i>=N) i = N-1;
    ind[--M] = i;
    h[i] = 0.0;
  }
  return 1;
}

bool multiple_selection(double *h, int N, int *ind, int M) {
  // all h[i] > 0 !
  // multiple occurance allowed
  if (!h || !ind || !N || !M) return 0;
  //if (!ind) ind = new int[M];
  int i;
  double sum = h[0];
  for (i=1; i<N; i++) sum += h[i];
  while (M>0) {
    double x = rnd(0.0, sum);
    for (i=0; i<N; i++) {
      if (x<h[i]) break;
      x -= h[i];
    }
    if (i>=N) i = N-1;
    ind[--M] = i;
  }
  return 1;
}

struct ranking_sort_type {
  double f;
  int index;
  ranking_sort_type() {f=0.0; index=0;}
  ranking_sort_type(double ff, int ii) {f=ff; index=ii;}
};

static int ranking_compare(const void *aa, const void *bb) {
  ranking_sort_type *a = (ranking_sort_type*)aa; 
  ranking_sort_type *b = (ranking_sort_type*)bb; 
  if (a->f < b->f) return -1;
  if (a->f > b->f) return +1;
  return 0;
}

bool ranking(double *h, int N, int *ind) {
  if (!h || !ind || !N) return 0;  
  // find maximum
  ranking_sort_type g[N];
  int i;
  for (i=0; i<N; ++i) {
    g[i].f = h[i];
    g[i].index = i;
  }   
  qsort(g, N, sizeof(ranking_sort_type), ranking_compare);
  for (i=0; i<N; ++i) ind[i] = g[N-i-1].index;
  return 1;
} 

static int index_compare(const void *aa, const void *bb) {
  int a = *(int*)aa, b = *(int*)bb;
  if (a<b) return -1;
  if (a>b) return +1;
  return 0;
}

void index_sort(int *ind, int N) {
  qsort(ind, N, sizeof(int), index_compare);
}

// CHROMOSOMES 

Chromosome::Chromosome(const Chromosome& s) {
  N = s.N;
  x = new double[N];
  for (int i=0; i<N; ++i) x[i] = s.x[i];
  l = s.l;
  u = s.u;
}
 
Chromosome::construct(int n) {
  N = n;
  x = new double[N];
  l = u = 0;
  fitn = 0.0;
}

Chromosome::construct(int n, double min, double max) {
  N = n;
  x = new double[N];
  l = new double[N];
  u = new double[N];
  for (int i=0; i<N; i++) { l[i] = min; u[i] = max; }
  fitn = 0.0;
}

Chromosome::construct(int n, double *min, double *max) {
  N = n;
  x = new double[N];
  l = new double[N];
  u = new double[N];
  for (int i=0; i<N; i++) {
    l[i] = min[i];
    u[i] = max[i];
  }
  fitn = 0.0;
}

Chromosome::~Chromosome() {
  if (x) delete x;
  if (l) delete l;
  if (u) delete u;
}
   
Chromosome& Chromosome::operator= (const Chromosome& s) {
  if (!N) N = s.N;
  if (!x) x = new double[N];
  if (!l) l = s.l;
  if (!u) u = s.u;
  int i, n = N<s.N ? N : s.N;
  for (i=0; i<n; ++i) x[i] = s.x[i];
  fitn = s.fitn;
  lastop = GAnop;
}

bool Chromosome::valid() {
  if (!x || !u || !l) return 0;
  for (int i=0; i<N; ++i) if (x[i]<l[i] || x[i]>u[i]) return 0;
  return 1;
}

void Chromosome::boundary() {
  for (int i=0; i<N; ++i) x[i] = rnd()>0.5 ? u[i] : l[i];
  fitn = 0.0;
  lastop = GAnop;
}
    
void Chromosome::dice() {
  for (int i=0; i<N; ++i) x[i] = rnd(l[i], u[i]);
  fitn = 0.0;
  lastop = GAnop;
}
    
void Chromosome::u_mutate() {
  // k = rnd(0,N-1);
  k = rnd(N);
  x[k] = rnd(l[k], u[k]);
  lastop = GAuniformMutation;
}

void Chromosome::b_mutate() {
  int k = rnd(N); // rnd(0,N-1);
  x[k] = rnd()>0.5 ? u[k] : l[k];
  lastop = GAboundaryMutation;
}

void Chromosome::n_mutate(double T) { 
  // sim. annealing: T = (1-(epoche/maxepoch))^b
  k = rnd(N); // rnd(0,N-1);
  double r = T*rnd();  
  x[k] = rnd()>0.5 ? (u[k]-x[k])*r : (x[k]-l[k])*r;
  lastop = GAnonuniformMutation;
}

void Chromosome::s_crossover(Chromosome* v) {
  // simple crossover
  int NN = (N>v->N ? N : v->N)-1;
  int q, i;
  for (q=0; q<trials; ++q) {
    k = rnd(N);
    Chromosome new_this = *this;
    Chromosome new_v = *v;
    for (i=k; i<N; ++i) new_this.x[i] = v->x[i];
    for (i=k; i<v->N; ++i) new_v.x[i] = x[i];
    if (new_this.valid && new_v.valid) {
      *this = new_this;
      *v = new_v;
      lastop = GAsimpleCrossover;
      return;
    }
  }
  // nothing changed
}


void Chromosome::a_crossover(Chromosome* v) {
  int q, i;
  for (q=0; q<trials; ++q) {
    double a = rnd();
    double b = 1.0-a;
    Chromosome new_this = *this;
    Chromosome new_v = *v;
    for (i=0; i<N; ++i) {
      new_this.x[i] = a*x[i] + b*v->x[i];
      new_v.x[i] = a*v->x[i] + b*x[i];
    }
    if (new_this.valid && new_v.valid) {
      *this = new_this;
      *v = new_v;
      lastop = GAarithmeticalCrossover;
      return;
    }
  }
  // nothing changed
}    

double Chromosome::d(Chromosome& s) {
  if (env.N && s.env.N) {
    int n = env.N<s.env.N?env.N:s.env.N;
    double y, sum = 0.0;
    for (int i=0; i<n; ++i) {
      y = env[i] - s.env[i];
      sum += y*y;
    }
    return sqrt(sum);
  }
  return 0.0;
}


ostream& operator <<(ostream& os, const Chromosome& s) {
  int i;
  switch (s.lastop) {
  defaults:
  case GAnop:
    os << "< " << s.x[0]; for(i=1; i<s.N; ++i) os << ' ' << s.x[i]; os << " >";
    break;
  case GAquality:
    os << "< " << s.x[0]; for(i=1; i<s.N; ++i) os << ' ' << s.x[i]; 
    os << ", " << s.fitn << " >";
    break;
  case GAuniformMutation: 
  case GAboundaryMutation: 
  case GAnonuniformMutation:
    if (s.k==0) os << "< [" << s.x[0] << ']'; 
    else os << "< " << s.x[0]; 
    for(i=1; i<s.N; ++i) {
      if (s.k==i) os << ' ' << '[' << s.x[0] << ']' ;
      else os << ' ' << s.x[i];
    } 
    os << " >";
    break;
  case GAsimpleCrossover: 
    if (s.k==0) os << "< [" << s.x[0]; 
    else os << "< " << s.x[0]; 
    for(i=1; i<s.N; ++i) {
      if (s.k==i) os << ' ' << '[' << s.x[0];
      else os << ' ' << s.x[i];
    } 
    if (s.k<s.N) os << "] >";
    else os << " >";
    break;
  case GAarithmeticalCrossover:
    os << "< [" << s.x[0]; 
    for(i=1; i<s.N; ++i) os << ' ' << s.x[i];
    os << "] >";
    break;
  }
}


// GENOTYPES

Genotype::Genotype() {
  epoche = 0;
}

Genotype::Genotype(Chromosome& s, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m; 
  par = new ChromosomePointer[2*mu];
  for (int i=0; i<2*mu; ++i) par[i] = new Chromosome(s);
  chi = par+mu;
  epoche = 0;
}

Genotype::Genotype(Chromosome* ps, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m; ; 
  par = new ChromosomePointer[2*mu];
  int i;
  for (i=0; i<mu; ++i) par[i] = ps+i;
  for (i=mu; i<2*mu; ++i) par[i] = new Chromosome(*ps);
  chi = par+mu;
  epoche = 0;
}

Genotype::~Genotype() {
}

void Genotype::reproduction(int ep) {
  if (ep>=0) epoche = ep;
  // create the next generation
  // parents
  test(cout << endl << "epoch " << epoche << endl);
  if (epoche>0) {
    int i, j;
    // probability for reprod.
    flag.regional = 1;
    if (flag.regional) {
      // marriage 
      double P[mu*mu];
      for (i=0; i<mu; ++i) {
	P[i*mu+i] = 0.0;
	for (j=i+1; j<mu; ++j) {
	  P[i*mu+j] = P[j*mu+i] = 
	    pow(par[i]->fitness()*par[j]->fitness()*corr(par[i],par[j]), p.a);
	}
      }
      int l, la2 = mu/2;
      int I[la2]; 
      single_selection(P, mu*mu, I, la2);
      for (l=0; l<la2; ++l) {
	*chi[2*l]   = *par[I[l]/mu];
	*chi[2*l+1] = *par[I[l]%mu];
      }
    }
    else {
      // find total fittness of population
      double F[mu];
      for (i=0; i<mu; ++i) F[i] = par[i]->fitness();
      int I[mu]; 
      multiple_selection(F, mu, I, mu);
      for (j=0; j<mu; ++j) *chi[j] = *par[I[j]];
    }
  }
  // manipulate childs
  nextgen();
}

void Genotype::nextgen() {
  if (epoche==0) {
    int i;
    int pi = int (p.prop*mu);
    for (i=0; i<pi; ++i) {
      chi[i]->dice();
      test(cout << "di " << i << ": " << *chi[i] << endl);
    }
    for (i=pi; i<mu; ++i) {
      chi[i]->boundary(); 
      test(cout << "bo " << i << ": " << *chi[i] << endl);
    }
  }
  else {
    int i, j;
    // crossover
    if (p.psc>0.0 || p.pac>0.0) 
      for (j=1; j<mu; j+=2) {
	if (rnd()<p.psc) {
	  chi[j]->s_crossover(chi[j-1]);
	  test(cout <<j-1<<'+'<<j<<'\t'<<*chi[j-1]<<"\n\t"<< *chi[j] << endl); 
	}
	if (rnd()<p.pac) {
	  chi[j]->a_crossover(chi[j-1]);
	  test(cout << j-1 << '+' << j << '\t' << *chi[j-1] << '\n'
	       << '\t' << *chi[j] << endl); 
	}
      }
    // mutation
    double T = pow(1.0-double(epoche)/p.maxepoch, p.b);
    if (p.pum>0.0 || p.pbm>0.0 || p.pnm>0.0)  
      for (j=0; j<mu; ++j) {
	test(cout << j << ": " << *chi[j]);
	if (rnd()<p.pum) {
	  chi[j]->u_mutate();
	  test(cout << "  um " << *chi[j]);
	}
	if (rnd()<p.pbm) {
	  chi[j]->b_mutate();
	  test(cout << "  bm " << *chi[j]);
	}
	if (rnd()<p.pnm) {
	  chi[j]->n_mutate(T);
	  test(cout << "  nm " << j << ": " << *chi[j]);
	}
	test(cout << endl); 
	// fittness
	//chi[j]->fitness(0.0);
      }
    // that's all
  }
}

void Genotype::survival() {
  // for lmabda = mu (as required) every individual survivaes
  double G[mu];
  int I[mu]; 
  int i;
  for (i=0; i<mu; ++i) G[i] = chi[i]->fitness();
  test(cout << "prob ");
  test(for (i=0; i<mu; ++i) cout << ' '  << G[i] << ',' << I[i]);
  test(cout << endl);
  ranking(G, mu, I);
  test(cout << "copy ");
  for (i=0; i<mu; ++i) {
    int a = I[i];
    test(cout << ' ' << i << "->" << a);
    *par[i] = *chi[I[i]];
  }
  test(cout << endl);
  besti = 0;
  epoche++;
}


Chromosome& Genotype::best() {
  if (besti<0 || besti>=mu) {
    besti = 0;
    for (int i=1; i<mu; ++i) 
      if (par[i]->fitness()>par[besti]->fitness()) besti = i;
  }
  return *par[besti];
}

double Genotype::corr(Chromosome& s1, Chromosome& s2) {
  return exp(-s1.d(s2)/p.b);
}

// GENOTYPE+

GenotypePlus::GenotypePlus(Chromosome& s, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m;
  par = new ChromosomePointer[2*mu];
  for (int i=0; i<2*mu; ++i) par[i] = new Chromosome(s);
  chi = par+mu;
  epoche = 0;
}

GenotypePlus::GenotypePlus(Chromosome* ps, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m;
  par = new ChromosomePointer[2*mu];
  int i;
  for (i=0; i<mu; ++i) par[i] = ps+i;
  for (i=mu; i<2*mu; ++i) par[i] = new Chromosome(*ps);
  chi = par+mu;
  epoche = 0;
}

GenotypePlus::~GenotypePlus() {
}

void GenotypePlus::survival() {
  // mu selections of mu+lambda
  int n = 2*mu;
  double G[n];
  int I[n]; 
  int i;
  for (i=0; i<n; ++i) G[i] = par[i]->fitness();
  test(cout << "prob ");
  test(for (i=0; i<mu; ++i) cout << ' '  << G[i] << ',' << I[i]);
  test(cout << endl);
  flag.fitest = 1;
  if (flag.fitest) {
    ranking(G, n, I);
  }
  else {
    single_selection(G, n ,I, mu); 
  }
  test(cout << "copy ");
  
//index sort !!
  index_sort(I, mu);
  *par[0] = *par[I[0]]; // works only if index is sorted!!!  eg. 0,5,6,9,... 
  for (i=1; i<mu; ++i) {
    test(cout << ' ' << i << "->" << I[i]);
    *par[i] = *par[I[i]]; // works only if index is sorted!!!  eg. 0,5,6,9,... 
  }
  test(cout << endl);
  besti = -1; // if you want the best, search it.
  epoche++;
}


// GenotypeCons

GenotypeCons::GenotypeCons(Chromosome& s, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m; 
  par = new ChromosomePointer[2*mu];
  for (int i=0; i<2*mu; ++i) par[i] = new Chromosome(s);
  chi = par+mu;
  epoche = 0;
  flag.regional = 1;
}

GenotypeCons::GenotypeCons(Chromosome* ps, int m, Parameters& pp) {
  // genotype wird mit eine muster erzeugt;
  p = pp;
  mu = m%2? m+1: m; ; 
  par = new ChromosomePointer[2*mu];
  int i;
  for (i=0; i<mu; ++i) par[i] = ps+i;
  for (i=mu; i<2*mu; ++i) par[i] = new Chromosome(*ps);
  chi = par+mu;
  epoche = 0;
  flag.regional = 1;
}


void GenotypeCons::survival() {
  // for lmabda = mu (as required) every individual survives
  double G[mu];
  int I[1]; 
  int i, j;
  for (j=0; j<mu; ++j) {
    for (i=0; i<mu; ++i) G[i] = chi[i]->fitness() * corr(par[j], chi[i]);
    single_selection(G, mu, I, 1);
    *par[j] = *chi[I[0]];
  }
  besti = -1;
  epoche++;
}


// Evolutionsstrategie

ESV::ESV() {
  x = 0;
  sigma = 0;
  N = 0;
  fitness = 0.0;
}

ESV::ESV(int n) {
  x = 0;
  sigma = 0;
  N = 0;
  fitness = 0.0;
  dim(n);
}

ESV::~ESV() {
  delete[] x;
  delete[] sigma;
}


ESV& ESV::operator= (const ESV& a) {
  if (a.N) {
    // assert(N==a.N);
    if(a.N!=N) {
      N = a.N;
      if (x) delete[] x;
      x = new double[N];
      if (sigma) delete[] sigma;
      sigma = new double[N];
    }
    for (int i=0; i<N; ++i) {
      x[i] = a.x[i];
      sigma[i] = a.sigma[i];
    }
    fitness = a.fitness;
  }
}

void ESV::dim(int n) {
  N = n;
  if (x) delete[] x;
  if (sigma) delete[] sigma;
  x = new double[N];
  sigma = new double[N];
  for (int i=0; i<N; ++i) {
    x[i] = rnd(-1.0,1.0);
    sigma[i] = rnd(0.1, 0.3); 
  }
}

bool ESV::valid() {
  for (int i=0; i<N; ++i)
    if (x[i]<-1.0 || x[i]>1.0 || sigma[i]<=0.0) return 0;
  return 1;
}

void ESV::mutate(double dsigma) {
  for (int i=0; i<N; ++i) {
    sigma[i] *= exp(rndg(0.0, dsigma));
    x[i] += rndg(0.0, sigma[i]);
  }
}

void ESV::crossover(ESV &a, ESV &b) {
  for (int i=0; i<N; ++i) {
    if (rnd(0.0,1.0)>0.5) {
      x[i] = a.x[i];
      sigma[i] = a.sigma[i];
    }
    else{
      x[i] = b.x[i];
      sigma[i] = b.sigma[i];
    }
  }
}

void ESV::intermediate(ESV &a, ESV &b) {
  for (int i=0; i<N; ++i) {
    x[i] = (a.x[i] + b.x[i])/2.0;
    sigma[i] = (a.sigma[i] + b.sigma[i])/2.0;
  }
}


ostream& operator <<(ostream& os, const ESV& s) {
  if (s.N && s.x && s.sigma) {
    int i;
    os << "(" << s.x[0]; 
    for(i=1; i<s.N; ++i) os << '|' << s.x[i]; 
    os << ", " << s.sigma[0];
    for(i=1; i<s.N; ++i) os << '|' << s.sigma[i]; 
    os << ", ";
    double f = s.fitness;
    os << f << ")";
  }
  else os << "(--NaC--)";
}



ES::ES() {
  par = 0;
  chi = 0;
  mu = lambda = 0;
  suc = 0;
}

ES::ES(int m, int l, int n) {
  int i;
  mu = m;
  lambda = l;
  par = new ESV[mu];
  for (i=0; i<mu; ++i) par[i].dim(n);
  chi = new ESV[lambda];
  for (i=0; i<lambda; ++i) chi[i].dim(n);
  suc = new double[k]; for (i=0; i<k; ++i) suc[i] = 0.0;
}


ES::~ES() {
  if (par) delete[] par;
  if (chi) delete[] chi;
}

void ES::reproduction(double tt, double ds) {
  // produce lambda  new individuals
  int J[2*lambda];
  double h[mu];
  int i;
  if (tt<0.000001) for (i=0 ;i<mu; ++i) h[i] = 1.0;
  else for (i=0 ;i<mu; ++i) h[i] = pow(par[i].fitness, tt); 
  multiple_selection(h, mu, J, 2*lambda);
  for (i=0; i<lambda; ++i) {
    chi[i].crossover(par[J[2*i]], par[J[2*i+1]]);
    chi[i].mutate(ds);
  }
}



void ES::select() {
  // comma-strategy
  double f = best().fitness;
  double G[lambda];
  int I[lambda]; 
  int i;
  for (i=0; i<lambda; ++i) G[i] = chi[i].fitness;
  //   test(cout << "prob ");
  //   test(for (i=0; i<lambda; ++i) cout << ' '  << G[i] << ',' << I[i]);
  //   test(cout << endl);
  ranking(G, lambda, I);
  //  test(cout << "copy ");
  for (i=0; i<mu; ++i) {
    //    test(cout << ' ' << i << "->" << I[i]);
    par[i] = chi[I[i%lambda]];
  }
  //  test(cout << endl);
  besti = 0;
  if (f>0.0 && best().fitness>f) { // erfolgreich
    suc[epoch%k] = (best().fitness-f)/f;
  }
  else suc[epoch%k] = 0.0;
}

void ES::selectplus() {
  // plus-strategy
  double f = best().fitness;
  double G[mu+lambda];
  ESV p[mu];
  int i;
  int I[mu+lambda]; 
  for (i=0; i<mu; ++i) {
    p[i] = par[i]; // zwischenspeichern
    G[i] = par[i].fitness;
  }
  for (i=0; i<lambda; ++i) G[mu+i] = chi[i].fitness;
  ranking(G, mu+lambda, I);
  for (i=0; i<mu; ++i) {
    int j = I[i];
    if (j<mu) par[i] = p[j];
    else par[i] = chi[j-mu];
  }
  besti = 0;
  if (f>0.0 && best().fitness>f) { // erfolgreich
    suc[epoch%k] = (best().fitness-f)/f;
  }
  else suc[epoch%k] = 0.0;
}

double ES::w() {
  double sum = 0.0;
  for (int i=0; i<k; ++i) sum += suc[i];
  return sum/double(k);
}

void ES::optimize(double tt, double ds, double me, 
		  double (*o)(double*)) {
  objective = o;
  while (me>0) {
    epoch++;
    reproduction(tt, ds);
    cout << epoch;
    for (int i=0; i<lambda; ++i) {
      if (!chi[i].valid()) chi[i].fitness = 0.0;
      else chi[i].fitness = (*o)(chi[i].x);
      cout << '\t' << chi[i] << endl;
    }
    selectplus();
    me--;
    cout << endl << '\t' << par[0] << endl << endl;
  }
}
      


#undef PROBIER
// Probier
#ifdef PROBIER
int probier_evolution () {
  double jojo = 50.0;
  double& peter = jojo;
  jojo = 100.0;
  cout << "peter=" << peter << endl;

  int i;
  cout << "test selection procedures" << endl;
  int I[10];
  cout << endl << "single  \t";
  double A[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  single_selection(A, 10,I, 10);
  for (i=0; i<10; ++i) cout << " " << I[i];
  cout << endl;

  cout << endl << "multiple\t";
  double B[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  double B2[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  multiple_selection(B, 10,I, 10);
  for (i=0; i<10; ++i) cout << " " << I[i];
  cout << endl;
  cout << "\t\t";
  multiple_selection(B2, 10,I, 10);
  for (i=0; i<10; ++i) cout << " " << I[i];
  cout << endl;
  
  cout << endl << "ranking \t";
  double C[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  double D[10] = {10.0, 4.0, 5.3, 2.5, 8.2, 1.9, 7.9, 8.3, 9.1, 1.2};
  if(ranking(C, 10,I)) {
    for (i=0; i<10; ++i) cout << " " << I[i];
  }
  cout << endl;
  cout << "\t\t";
  if (ranking(D, 10,I)) {
    for (i=0; i<10; ++i) cout << ' ' << I[i];
  }
  cout << endl;

  int I[] = {7, 5, 3, 9, 4, 2, 8, 6, 1, 0};
  index_sort(I, 10);
  cout << "sort";
  for (int jj=0; jj<10; ++jj) cout << ' ' << I[jj];
  cout << endl;

 
}
#endif





