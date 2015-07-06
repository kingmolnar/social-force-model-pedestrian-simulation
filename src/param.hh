#ifndef _PARAM_HH_
#define _PARAM_HH_

#include <stdio.h>
#include <iostream.h>
#include <math.h>
#include "vector.hh"
#include "list.hh"
#include "hardware.h"

typedef struct _WidgetRec *Widget;
extern "C" {
#define new new_
#define class class_
#undef new
#undef class
}

class fixed1 { // 0..+1
  unsigned int val : 8;
public:
  fix1() {val=0;}
  fix1(float x) {x *= 255.0; val = (unsigned int)(x);}
  operator float() {return float(val)/255.0;}
};

class fixed2 { // -1..+1
  int val : 16;
public:
  fix2() {val=0;}
  fix2(float x) {x *= 32767.0; val = int(x);}
  operator float() {return float(val)/32767.0;}
};

class fixed4 { // -2e5..+2e5 
  int val : 32;
public:
  //fixed4() {val=0;}
  //  fix4(float x) {x*= 214748.3647; val = int(x);}
  //operator float() {return float(val)/214748.3647;}
  //fixed4(float x) {x*= 10000.0; val = int(x);}
  operator float() {return float(val)/10000.0;}
  operator=(float x) {x*= 10000.0; val = int(x);}
};


double rnduniform(double a=0.0, double b=1.0);
//double rnduniform() {return rnduniform(0.0,1.0);}
int rnduniform(int k, int l);

double rndgauss(double, double);
// h=vector of probabilities, n=number, returns one selection and sets h_i = 0
int irndsel(double *h, int n);

class RealFunction {
public:
  virtual operator double() = 0;
};

class RndUniform: public RealFunction {
public:
  double a, b;
  RndUniform(double _a, double _b) {a = _a; b = _b;}
  operator double() {return rnduniform(a, b);}
};

class RndGauss: public RealFunction {
public:
  double a, b, min, max;
  RndGauss(double _a, double _b) {a = _a; b = _b;}
  operator double() {return rndgauss(a, b);}
};


class Matrix {
  double *val;
  int NN, MM;
public:
  Matrix(int n, int m);
  Matrix(const Matrix& b);
  ~Matrix();
  double& operator() (int i, int j) {return val[i*MM+j];}
  operator= (const Matrix& b);
  operator= (double v);
  int N() {return NN;}
  int M() {return MM;}
};


class Data { // auch mehrere Variablen
private:
  double *X, *Xp;
  unsigned int n, dim;
public:
  Data();
  Data(int);
  ~Data();
  void clear();
  void data(double);
  void data(double, int);
  void data(double*);
  void data(double*, int);
  double mean(int a = 0) {return X[a]/n;}
  double var();
  double var(int a, int b=-1);
  double std() {return sqrt(var());}
  double std(int a) {return sqrt(var(a,a));}
  double rho(int a, int b) {return var(a,b)/(std(a)*std(b));}
  unsigned int N() const {return n;}
  double *calc();
};
typedef Data* DataPointer;

ostream& operator<< (ostream&, Data);

class Prognosis {
private:
  const double NaN = 0.0;
  int to_calc;
  int v;
  int m;
  int number, min_number;
  Matrix *a; 
  Matrix *b;
  Matrix *covar;
  Matrix *beta;
public:
  Prognosis(int n, int mini);
  ~Prognosis();
  void clear();
  void data(double x, double y, double s);
  double c(int i=0) {
    if (to_calc) prognosis(); return valid() ? (*beta)(i,0) : NaN;}
  double cvar(int i=0) {
    if (to_calc) prognosis(); return valid() ? (*covar)(i,i) : NaN;}
  double cvar(int i, int j) {
    if (to_calc) prognosis(); return valid() ? (*covar)(i,j) : NaN;}
  int prognosis();
  //int valid() const {return (min_number && number>=min_number && v);}
  int valid() const {return ((min_number>0) && (number>=min_number) 
			     && (to_calc==0));}
  int M() const {return m;}
  int N() const {return number;}
};

typedef Prognosis* PrognosisPointer;

//void gaussj(Matrix& a, Matrix& b);

class Parameter;
enum ParameterValueType {NIL, _real, _int, _str, _vec, _intvec, _bool,
			 FNCTN, _runi, _rgau}; 

struct ParameterDef {
  char *id, *definition;
};

class ParameterValue {
friend class Parameter;
public:
  char* id;
  ParameterValueType type;
  union {
    int i;
    double d;
    IntVector *iv;
    Vector *v;
    char *s;
    bool b;
    RealFunction *fct;
  } value;
  ParameterValue *next;
  bool changed;
public:
  ParameterValue();
  ~ParameterValue();
  operator = (double v);
  operator = (int v);
  operator = (char *v);
  operator = (Vector& v);
  operator = (IntVector& v);
  operator = (bool v);
  operator = (ParameterValue&);

  operator double(); 
  operator int() {return value.i;}
  operator char*() {return value.s;}
  operator Vector() {return *value.v;}
  operator IntVector() {return *value.iv;}
  operator bool() {return value.b;}
  
  int scan(char*);
  void clear();
};
#define PVAL (int)


class Parameter {
  //friend void Simul::parse(FILE*);
  ParameterValue *list;
public:
  Parameter();
  Parameter(ParameterDef* d, int n);
  void insert(ParameterDef* d, int n);
  ParameterValue* create(char*);
  ParameterValue* resolve(char*);
  ParameterValue* rresolve(char*);
  Parameter *parent;
  
  Vector *v;
  double *a;
  bool *b;
  int nv, na, nb;
  
  int flags;
  
  int set(char *r, double d);
  int set(char *r, int i);
  int set(char *r, bool b);
  int set(char *r, Vector v);
  int set(char *r, char* s);
  int get(char *R, double *A);
  int get(char *R, int *A);
  int get(char *R, bool *A);
  int get(char *R, Vector *A);
  int get(char *R, char **A);  
  bool changed(char *R); // nur fuer direkte Referenz 
  bool flag(int i) { return (i>=0 && i<nb && b[i]); }
  virtual int read(FILE*);
  virtual int write(FILE*);
  //virtual int write() {return write(stdio);}
};

template <class ParameterDataType>
void get(Parameter *p, char *R, ParameterDataType *A) {
  if (!p->get(R, A)) {
    fprintf(stdout, "Parameter `%s': not defined\n", R);
  }
}

// old style og get(...) defaults will be created
template <class ParameterDataType>
void get(Parameter *p, char *R, ParameterDataType *A, ParameterDataType V) {
  if (!p || !p->get(R, A)) {
    *A = V;
    ParameterValue *pv = p? p->create(R): 0;
    if (pv) *pv = V;
  }
} 

/* template <class ParameterDataType>
   void get(Parameter *p, char *R, ParameterDataType *A, ParameterDataType V) {
   if (!p || !p->get(R, A)) {
   *A = V; // ParameterValue *pv = p? p->create(R): 0; //if (pv) *pv = V;
  }
  } */

template <class ParameterDataType>
void def(Parameter *p, char *R, ParameterDataType V) {
  if (p) {
    ParameterValue *pv = p->create(R);
    if (pv) *pv = V;
  }
}

class WithParameter: public ListElement {
public:
  Parameter *parameter;
  WithParameter() { parameter = new Parameter;}
  virtual void init() {}
  virtual void panel(int, Widget, Widget, char*);
  virtual void panelCreate(Widget a, Widget b, char *s)   {panel(1, a, b, s);}
  virtual void panelValue(Widget a, Widget b, char *s)    {panel(2, a, b, s);}
  virtual void panelActivate(Widget a, Widget b, char *s) {panel(3, a, b, s);}
};

#endif 
//------ EOF ------














