#ifndef _EVOLUTION_HH_
#define _EVOLUTION_HH_

#include <iostream.h>

enum ChromosomeOp {GAnop,GAquality, 
		   GAuniformMutation,GAboundaryMutation,GAnonuniformMutation, 
		   GAsimpleCrossover,GAarithmeticalCrossover};

class Chromosome {
friend ostream& operator <<( ostream& , const Chromosome& );
  ChromosomeOp lastop;  
  int k;
  double fitn;
  const int trials = 5;
protected: 
  construct(int n);
  construct(int n, double min, double max);
  construct(int n, double *min, double *max);
public: 
  struct Env {
    double *y;
    int N;
    Env() {y=NULL; N=0;}
    double& operator[](int i) {return y[i];}
  } env;
  int N;
  double *x;
  double *l;
  double *u;
  double fitness() {return fitn;}
  double fitness(double f) {fitn = f; lastop = GAquality; return fitn;}
  Chromosome() {N = 0; x = l = u = 0;}
  Chromosome(const Chromosome& S);
  Chromosome(int n) {construct(n);}
  Chromosome(int n, double min, double max) {construct(n, min, max);}
  Chromosome(int n, double *min, double *max) {construct(n, min, max);}
  ~Chromosome(); 
  double& operator[](int i) {return x[i];}
  Chromosome& operator= (const Chromosome& s);
  //methodes
  bool valid();
  void boundary();
  void dice();
  void u_mutate();
  void b_mutate();
  void n_mutate(double);
  void s_crossover(Chromosome*);
  void a_crossover(Chromosome*);
  double d(Chromosome& s);
};

typedef Chromosome* ChromosomePointer;
ostream& operator <<(ostream& os, const Chromosome& s);

class Genotype { // (mu,labda)-ES, habloid
protected: 
  int  besti;
public: 
  union {
    struct {
      unsigned int plus : 1; // 1=(mu+mu)-ES, 0=(mu,mu)-ES
      unsigned int fitest : 1; // 1=select the fitest, 0=roulette
      unsigned int regional : 1;
      unsigned int sort : 1;
      unsigned int strategy : 4;
    } flag;
    int intflag;
  };
  int mu;
  int epoche;
  // parameters
  struct Parameters {
    double a, b, prop; // for non-uniform mutation
    double psc, pac;
    double pum, pbm, pnm;
    double maxepoch;
  } p;
  ChromosomePointer *par;
  ChromosomePointer *chi;
  Genotype();
  Genotype(Chromosome& s, int m, Parameters& pp); 
  Genotype(Chromosome* ps, int m, Parameters& pp); 
  ~Genotype();
  Chromosome& operator[](int i) {return *chi[i];}
  virtual void reproduction(int ep=-1);
  virtual void nextgen();
  virtual void survival();
  Chromosome& best();
  double corr(Chromosome* p1, Chromosome *p2) {return corr(*p1, *p2);}
  double corr(Chromosome& s1, Chromosome& s2);
};

class GenotypePlus: public Genotype { // (mu+labda)-ES
public: 
  GenotypePlus(Chromosome& s, int m, Parameters& pp); 
  GenotypePlus(Chromosome* ps, int m, Parameters& pp); 
  ~GenotypePlus();
  void survival();
};

class GenotypeCons: public Genotype {
public: 
  GenotypeCons(Chromosome& s, int m, Parameters& pp); 
  GenotypeCons(Chromosome* ps, int m, Parameters& pp);
  void survival();
};

// Evolutionsstrategie

class ESV {
friend ostream& operator <<( ostream& , const Chromosome& );
public:
  int N;
  double *x, *sigma;
  double fitness;
  ESV();
  ESV(int);
  ~ESV();
  ESV& operator= (const ESV& a);
  void dim(int);
  bool valid();
  void mutate(double);
  void crossover(ESV &a, ESV &b);
  void intermediate(ESV &a, ESV &b);
};

ostream& operator <<(ostream& os, const ESV& s);

class ES {
public:
  int mu, lambda;
  int epoch, besti;
  double dsigma, t;
  int maxepoch;
  double (*objective)(double*);
  double *suc;
  const int k = 10;
  double w();
  double cd, ci;
  ESV *par;
  ESV *chi;			       
  ESV& best() {return par[besti];}
  ES();
  ES(int,int,int);
  ~ES();
  void select();
  void selectplus();
  void reproduction(double,double);
  void optimize(double _t, double _ds, double _me, 
		double (*o)(double*)); 
  
};

#endif










