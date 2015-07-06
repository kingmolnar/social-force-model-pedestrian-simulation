#define NTEST
#include "test.h"
#include "assert.h"
//#include <iostream.h>
#include "param.hh"

#include <stdlib.h>
#include <math.h>

const bool True = 1;
const bool False = 0; 

extern "C" {
  int strcasecmp(char*, char*);
  char *strncpy(char*, char*, unsigned int);
  char *index(char *s, char c);
#undef IMSL
#ifdef IMSL
#ifdef NO_UNDERSCORES
  extern  double drnnof();
  extern  void rnset(), rnopt();
#define DRNNOF drnnof
#define RNSET rnset
#define RNOPT rnopt
#else //  NO_UNDERSCORES
  extern  double drnnof_();
  extern  void rnset_(), rnopt_();
#define DRNNOF drnnof_
#define RNSET rnset_
#define RNOPT rnopt_
#endif //  NO_UNDERSCORES
#endif
  //long random();
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

float ran1(long *idum)
{
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


float gasdev(long *idum)
{
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


// zufallszahlen mit imsl
#ifdef IMSL
int seed=0, opt=1;
static int randset = 0;
#endif


double rnduniform(double a, double b) {
  //  return (b - a)*random()/2147483647.0 + a;
  static long idum = -1931;
  return (b - a)*ran1(&idum) + a;
}

//double rint(double);

inline int rnduniform(int k, int l) {
  return int(floor(rnduniform(double(k), double(l))+0.5));
}

double rndgauss(double a, double b) {
  static long idum = -1932;
#ifdef IMSL
  if (!randset) {
    randset = 1;
    RNSET(&seed);
    RNOPT(&opt);
  }
  double x = a + b * DRNNOF();
  //printf("norm %lf, %lf -> %lf\n", a, b, x);
  return x;
#else
  return a + b * gasdev(&idum);
#endif
}

int irndsel(double *h, int n) {
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

Data::Data() {
  dim = 1;
  X = new double;
  Xp = new double;
  *X = *Xp = 0.0;
  n = 0;
}

Data::Data(int i) {
  dim = i>0? i: 1;
  X = new double[dim];
  Xp = new double[dim*dim];
  int a;
  for (a=0; a<dim; ++a) X[a] = 0.0;
  for (a=0; a<dim*dim; ++a) Xp[a] = 0.0;
  n = 0;
}

Data::~Data() {
  delete X;
  delete Xp;
}

void Data::clear() {
  int a;
  for (a=0; a<dim; ++a) X[a] = 0.0;
  for (a=0; a<dim*dim; ++a) Xp[a] = 0.0;
  n = 0;
}

void Data::data(double y) {
  *X += y;
  *Xp += y*y;
  n++;
}

void Data::data(double y, int k) {
  double yk = y * k;
  *X  += yk;
  *Xp += y*yk;
  n += k;
}

void Data::data(double *y) {
  if (!y) return;
  for (int a=0; a<dim; ++a) {
    X[a] += y[a];
    for (int b=0; b<dim; ++b) Xp[a*dim+b] += y[a]*y[b];
  }
  n++;
}

void Data::data(double *y, int k) {
  if (!y) return;  
  double yk;
  for (int a=0; a<dim; ++a) {
    yk = y[a]*k;
    X[a]  += yk;
    for (int b=0; b<dim; ++b) Xp[a*dim+b] += yk*y[b];
  }
  n += k;
}

double Data::var() {
  double EX = mean();
  return *Xp/n - EX*EX;
}

double Data::var(int a, int b) {
  b = b<0 ? a: b;
  double EX = mean(a);
  double EY = mean(b);
  return Xp[a*dim+b]/n - EX*EY;
}

double *Data::calc() { // double **F) {
  int a, b, n2 = dim*dim, n3 = dim+n2;
  // if (*F==NULL) {
  double *F = new double[n3];
  // means F[0..dim]
  for (a=0; a<dim; ++a) F[a] = X[a]/n;
  // covarianz F[dim...dim+dim**2]
  int i;
  for (a=0; a<dim; ++a)
    for (b=0; b<dim; ++b) {
      i = a*dim+b;
      F[dim+i] = Xp[i]/n - F[a] * F[b];
    }
  return F;
}
		       

ostream& operator<< (ostream& s, Data d) {
  return s << "mean="<< d.mean() << " var=" << d.var();
}


#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>

Matrix::Matrix(int n, int m) {
  NN = n; MM = m; val = new double[NN*MM];
}

Matrix::Matrix(const Matrix& b) {
  NN = b.NN; MM = b.MM;
  int x = NN*MM;
  val = new double[x];
  for (int i=0; i<x; ++i) val[i] = b.val[i];
} 

Matrix::~Matrix() {delete[] val;}

Matrix::operator= (const Matrix& b) {
  if (NN*MM>0) delete[] val; 
  NN = b.NN; MM = b.MM;
  int x = NN*MM;
  val = new double[x];
  for (int i=0; i<x; ++i) val[i] = b.val[i];
}

Matrix::operator= (double v) {for (int i=0; i<NN*MM; ++i) val[i] = v;}

Prognosis::Prognosis(int _m, int mini) {
  m = _m<2 ? 2 : _m;
  a = new Matrix(m,m);
  b = new Matrix(m,1);
  covar = new Matrix(m,m);
  beta = new Matrix(m,1);
  clear();
  number = 0;
  min_number = mini;
}

Prognosis::~Prognosis() {
  delete a;
  delete b;
  delete covar;
  delete beta;
}

void Prognosis::clear() {
  for (int k=0; k<m; ++k) {
    for (int l=0; l<=k; ++l) (*a)(k,l) = (*a)(l,k) = 0.0;
    (*b)(k,0) = 0.0;
  }
  number = 0;
  v = 0; // invalid;
  to_calc = 1; 
}

void Prognosis::data(double x, double y, double variance) {
  int i, k, l;
  double f[m];
  f[0] = 1.0;
  for (i=1; i<m; ++i) f[i] = f[i-1]/x; 
  for (k=0; k<m; ++k) {
    for (l=0; l<=k; ++l) {
      (*a)(k,l) += f[k] * f[l] / variance; 
    }
    (*b)(k,0) += y * f[k] / variance;
  }
  number++;
  v = 0; // invalid;
  to_calc = 1; 
}

static void gaussj(Matrix& a, Matrix& b);


int Prognosis::prognosis() {
  //  if (!to_calc) return v; 
  to_calc = 0;
  if (number<min_number) {
    //    v = 0;  to_calc = 1;
    return 0; 
  }
  else {
    // auffuellen;
    int k,l;
    for (k=0; k<m; ++k) for (l=0; l<k; ++l) (*a)(l,k) = (*a)(k,l);
    *covar = *a;
    *beta = *b;
    gaussj(*covar, *beta);
    // v = 1;     to_calc = 0;
    return 1;
  }
}

static void SWAP(double& a, double& b) {double temp=(a);(a)=(b);(b)=temp;}   

/* (C) Copr. 1986-92 Numerical Recipes Software 5.){2ptNsz4'>4. */

void gaussj(Matrix& a, Matrix& b) {
  if (!a.N() || a.N()!=a.M() || a.N()!=b.N() || !b.M()) return;
  int n = a.N();
  int m = b.M();
  //  int *indxc,*indxr,*ipiv;
  int i,icol,irow,j,k,l,ll;
  double big,dum,pivinv,temp;
  
  int indxc[n];  // - 1; 
  int indxr[n];  //  - 1;
  int ipiv[n];   // - 1;
  for (j=0; j<n; j++) ipiv[j] = 0; // ipiv[j]=0;
  for (i=0; i<n; i++) {
    big=0.0;
    for (j=0; j<n; j++) {
      if (ipiv[j] != 1) { // if (ipiv[j] != 1)  
	for (k=0;k<n;k++) {
	  if (ipiv[k] == 0) { // if (ipiv[k] == 0) {
	    if (fabs(a(j,k)) >= big) {
	      big=fabs(a(j,k));
	      irow=j;
	      icol=k;
	    }
	  }
	  else if (ipiv[k] > 1) cerr << "gaussj: Singular Matrix-1" << endl;
	}
      }
    }
    ++(ipiv[icol]);
    if (irow != icol) {
      for (l=0;l<n;l++) SWAP(a(irow,l),a(icol,l));
      for (l=0;l<m;l++) SWAP(b(irow,l),b(icol,l));
    }
    indxr[i]=irow;
    indxc[i]=icol;
    if (a(icol,icol) == 0.0) cerr << "gaussj: Singular Matrix-2" << endl;
    pivinv=1.0/a(icol,icol);
    a(icol,icol)=1.0;
    for (l=0;l<n;l++) a(icol,l) *= pivinv;
    for (l=0;l<m;l++) b(icol,l) *= pivinv;
    for (ll=0;ll<n;ll++) {
      if (ll != icol) {
	dum=a(ll,icol);
	a(ll,icol)=0.0;
	for (l=0;l<n;l++) a(ll,l) -= a(icol,l)*dum;
	for (l=0;l<m;l++) b(ll,l) -= b(icol,l)*dum;
      }
    }
  }
  for (l=n-1;l>=0;l--) {
    if (indxr[l] != indxc[l])
      for (k=0;k<n;k++)
	SWAP(a(k,indxr[l]),a(k,indxc[l]));
  }
}

//-----------------------------

ParameterValue::ParameterValue() {
  id = 0;
  next = 0;
  type = NIL;
}

ParameterValue::~ParameterValue() {
  if (next) delete next;
  if (id) delete id;
}

Parameter::Parameter() {
  list = NULL;
  v = NULL;
  a = NULL;
  b = NULL;
  nv = na = nb = 0;
  parent = NULL;
}

Parameter::Parameter(ParameterDef* d, int n) {
  list = NULL;
  v = NULL;
  a = NULL;
  b = NULL;
  nv = na = nb = 0;
  parent = NULL;
  insert(d, n);
}

void Parameter::insert(ParameterDef* d, int n) {
  ParameterValue *v;
  for (int i=0; i<n; i++) {
    v = create(d[i].id);
    if (!v) return; // something wrong
    v->scan(d[i].definition);
  }
}

ParameterValue* Parameter::create(char *r) {
  test(printf("create \'%s\' ...", r));
  ParameterValue *p;
  if (p = resolve(r), p) return p;
  p = new ParameterValue;
  int len = strlen(r) + 1;
  p->id = new char[len];
  strncpy(p->id, r, len); p->id[len-1] = '\0';
  p->type = NIL;
  p->next = list;
  list = p;
  announce("... create done.\n");
  return p;
}

ParameterValue* Parameter::resolve(char *r) {
  test(printf("resolve \'%s\' ...", r));
  for (ParameterValue *l=list; l; l=l->next) {
    test(printf("%s ", l->id)); fflush(stdout);
    if (strcasecmp(r, l->id)==0) {
      test(printf("matched with \'%s\' %d\n", l->id, l->type));
      return l;
    }
  }
  test(printf(" not found.\n"));
  return NULL;
}

ParameterValue* Parameter::rresolve(char *r) {
  Parameter *p;
  ParameterValue *v;
  int i = 0;
  test(printf("rresolve \'%s\' ...\n", r));
  for (p=this; p; p=p->parent) {
    test(printf("level %d: ", i++));
    v = p->resolve(r);
    if (v && v->type!=NIL) return v;
  }
  test(printf(" realy not found.\n"));  
  return NULL;
}

//SET
int Parameter::set(char *r, double d) {
  ParameterValue *p = resolve(r);
  if (p) {
    *p = d;
    p->changed = True;
  }
  return (int)p;
}

int Parameter::set(char *r, int i) {
  ParameterValue *p = resolve(r);
  if (p) {
    *p = i;
    p->changed = True;
  }
  return (int)p;   
}

int Parameter::set(char *r, bool b) {
  ParameterValue *p = resolve(r);
  if (p) {
    *p = b;
    p->changed = True;
  }
  return (int)p;   
}

int Parameter::set(char *r, Vector v) {
  ParameterValue *p = resolve(r);
  if (p) {
    *p = v;
    p->changed = True;
  }
  return (int)p;
}

int Parameter::set(char *r, char* s) {
  ParameterValue *p = resolve(r); 
  if (p) {
    *p = s;
    p->changed = True;
  }
  return (int)p;
}
// GET
int Parameter::get(char *r, double *pd) {
  ParameterValue *p = rresolve(r);
  if (p && pd) {
    *pd = *p;
    p->changed = False;
    return (int)p;
  }
  else return 0;
}

int Parameter::get(char *r, int *pi) {
  ParameterValue *p = rresolve(r);
  if (p && pi) {
    *pi = *p;
    p->changed = False;
    return (int)p;
  }
  else return 0;
}

int Parameter::get(char *r, bool *pb) {
  ParameterValue *p = rresolve(r);
  if (p && pb) {
    *pb = *p;
    p->changed = False;
    return (int)p;
  }
  else return 0;
}

int Parameter::get(char *r, Vector *pv) {
  ParameterValue *p = rresolve(r);
  if (p && pv) {
    *pv = *p;
    p->changed = False;
    return (int)p;
  }
  else return 0;
}

int Parameter::get(char *r, char **pc) {
  ParameterValue *p = rresolve(r);
  if (p && pc) {
    *pc = *p;
    p->changed = False;
    return (int)p;
  }
  else return 0;
}

bool Parameter::changed(char *r) {
  ParameterValue *p = resolve(r);
  if (!p) return False;
  return p->changed;
}

int Parameter::read(FILE *fp) {
  int n;
  return n;
}

int Parameter::write(FILE* fp) {
  int i = 0;
  for (ParameterValue *l=list; l; l=l->next) {
    i++;
    fprintf(fp, "%s\t=\t", l->id);
    switch(l->type) {
    case _real:
      fprintf(fp, "%lf", l->value.d);
      break;
    case _int:
      fprintf(fp, "%d", l->value.i);
      break;
    case _str:
      fprintf(fp, "%s", l->value.s);
      break;
    case _vec:
      fprintf(fp, "%lf|%lf", l->value.v->x, l->value.v->y);
      break;
    case _intvec:
      fprintf(fp, "%d|%d", l->value.iv->x, l->value.iv->y);
      break;
    case _bool:
      if (l->value.b) fprintf(fp, "TRUE");
      else  fprintf(fp, "FALSE");
      break;
    case _runi:
      { 
	RndUniform *f = (RndUniform*)l->value.fct;
	fprintf(fp, "unif(%lf,%lf)", f->a, f->b);
      }
      break;
    case _rgau:
      { 
	RndGauss *g = (RndGauss*)l->value.fct;
	fprintf(fp, "norm(%lf,%lf)", g->a, g->b);
      }
      break;
    default:
      fprintf(fp, "./.");
      break;
    }
    fprintf(fp, ";\n");
  }
  return i;
}

ParameterValue::operator = (ParameterValue& pv) {
  type = pv.type; changed = 1;
  switch(pv.type) {
  case _real: 
    value.d = pv.value.d;
    type = _real;
    break;
  case _int:
    value.i = pv.value.i;
    type = _int;
    break;
  case _str:
    if (type==_str) delete value.s;
    if (type==NIL || type==_str) {
      int n = strlen(pv.value.s)+1;
      value.s = new char[n];
      strncpy(value.s, pv.value.s, n); type = _str;
    } 
    break;
  case _vec:
    if (type==_vec) delete value.v;
    if (type==NIL || type==_vec) {
      value.v = new Vector;
      *(value.v) = *(pv.value.v); type = _vec;
    }
    break;
  case _intvec:
    if (type==_intvec) delete value.v;
    if (type==NIL || type==_intvec) {
      value.iv = new IntVector;
      *(value.iv) = *(pv.value.iv); type = _intvec;
    }
    break;
  default:
    cerr << "exception in `ParameterValue::operator=(ParameterValue&)\'" 
      << endl;
    break;
  }
}


ParameterValue::operator = (double v) {
  value.d = v; type = _real;
}

ParameterValue::operator = (int v) {
    value.i = v; type = _int;
}

ParameterValue::operator = (bool v) {
  if (type==NIL || type==_bool) {
    value.i = v; type = _int;
   }
}

ParameterValue::operator = (char *v) {
  if (type==_str) delete value.s;
  if (type==NIL || type==_str) {
    int n = strlen(v)+1;
    value.s = new char[n];
    strncpy(value.s, v, n); type = _str;
  } 
}

ParameterValue::operator = (Vector& v) {
  if (type==_vec) delete value.v;
  if (type==NIL || type==_vec) {
    value.v = new Vector;
    *(value.v) = v; type = _vec;
  }
}

ParameterValue::operator = (IntVector& v) {
  if (type==_intvec) delete value.v;
  if (type==NIL || type==_intvec) {
    value.iv = new IntVector;
    *(value.iv) = v; type = _intvec;
  }
}

ParameterValue::operator double()  {
  switch (type) {
  case _real:
  default:
    return value.d;
    break;
  case _rgau:
  case _runi:
    return double(*value.fct);
    break;
  }
}
void ParameterValue::clear() {
  switch (type) {
    /* case _real:
       value.d = 0.0;
       break;
       case _int:
       value.i = 0;
       break; */
  case _str:
    delete value.s;
    value.s = 0;
    break;
  case _vec:
    delete value.v;
    value.v = 0;
    break;
  case _intvec:
    delete value.iv;
    value.iv = 0;
    break;
  }
  type = NIL;
}  

int ParameterValue::scan(char *text) {
  int sn = strlen(text);
  char stext[sn+1]; 
  strcpy(stext, text);
  stext[sn] = '\0';
  int n, good = 0, i, j;
  double  a, b, c, d;
  clear();
  //test(printf("Typ finden ... ")); 
  if (*text=='"' || *text=='\'') type = _str;
  else if (index(text, '(') && index(text, ')')) type = FNCTN;
  else if (*text=='$') type = NIL;
  else if (strcasecmp(text,"true")==0 || strcasecmp(text,"false")==0) 
    type = _bool;
  else {
    if (index(text, '|')) {
      if (index(text, '.')) type = _vec;
      else type = _intvec;
    }
    else { // kein Vector
      if (index(text, '.')) type = _real;
      else type = _int;
    }
  }  
  //test(printf("%d\n", type)); 
  switch (type) {
  case _real:
    good = sscanf(stext, "%lf", &a);
    value.d = a;
    //value.d = atof(text);
    break;
  case _int:
    good = sscanf(stext, "%d",  &i);
    value.i = i;
    break;
  case _bool:
    if (strcasecmp(text,"true")==0) value.b = 1;
    else value.b = 0;
    good =1;
    break;
  case _str:
    if (*text=='\'') { 
      n = strlen(++text)+1;
    }
    else if(*text=='"') {
      n = strlen(++text);
    }
    else n = strlen(text); // erstes zeichen ' ' oder " "  wird weggeschm.
    value.s = new char[n];
    if (value.s) {
      strncpy(value.s, text, n-1);
      value.s[n] = '\0';
      good = 1;
    }
    else good = 0;
    break;
  case _vec:
    value.v = new Vector;
    good = sscanf(text, "%lf|%lf", &a, &b);
    value.v->x = a; value.v->y = b;
    break;
  case _intvec:
    value.iv = new IntVector;
    good = sscanf(text, "%d|%d", &i, &j);
    value.iv->x = i; value.iv->y = j;
    break;
  case FNCTN: // teste ob funktion
    if (sscanf(text, "unif(%lf,%lf)",&a, &b)) {
      type = _runi;
      value.fct = new RndUniform(a, b);
      good = 1;
    }
    else if (sscanf(text, "norm(%lf,%lf)",&a, &b)) {
      type = _rgau;
      value.fct = new RndGauss(a, b);
      good = 1;
    }
    break;
  }
  changed = True;
  //test(printf("good=%d\n", good));
  if (!good) type = NIL;
  return good;
}
