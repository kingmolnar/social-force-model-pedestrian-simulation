#ifndef _SIMUL_H_
#define _SIMUL_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>
#include "vector.hh"
#include "list.hh"
#include "param.hh"

#include "endian.hh"

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef NEED_SETENV
int setenv(char*, char*, int);
#else
extern "C" int setenv(char*, char*, int);
#endif



#include "simultypes.hh"

enum SimulObjectTasks { UNDEF, INIT, SET_VALUES, RUN, RESET,
			SLEEP, TERMINATE, DEACTIVATE };

class SimulObject: public WithParameter, public WithMessage {
public: 
  List refs;
  SimulObject *parent;
  SimulObject();
  virtual SimulObjectType objecttype(int i = 0) {return _UNDEF;}
  int operator>=(const SimulObjectType);
  SimulObjectTasks objecttask;
  // the five methodes
  void init();
  virtual int ref(SimulObject *obj, SimulObjectType t = _UNDEF); // {return 0;}
  virtual int update() {return 0;}
  virtual void clock() {}
  /* virtual */ SimulObject* clone();

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
  char* def;
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
  WithParameter();
  ~WithParameter();
  virtual void init(); 
  virtual void panel(int, Widget, Widget, char*);
  virtual void panelCreate(Widget a, Widget b, char *s);
  virtual void panelValue(Widget a, Widget b, char *s);
  virtual void panelActivate(Widget a, Widget b, char *s);
};

#endif 
//------ EOF ------









 // Binary IO -> file, socket ...
  virtual unsigned int queryiosize() {return 0;}
  virtual int write(int) {return 0;}
  virtual int read(int) {return 0;}
  virtual int read(int f, unsigned int bytes) {return read(f);}
};

typedef SimulObject* SimulObjectPointer;

class SimulObjectNode {
public: 
  SimulObjectNode *parent, *next, *tree;
  SimulObject *obj;
  char *id;
  char defined;
  SimulObjectType type;
  List creators;
  SimulObjectNode(SimulObjectNode*, SimulObjectType, char*);
  SimulObjectNode *reference(SimulObjectType, char*);
  SimulObjectNode *reference(char*);
  void parse(TextFile*);
  operator SimulObject*() {return obj;}
  int ObjectType(char *token, SimulObjectType *t);
  SimulObject* CreateObject(SimulObjectType y);
};

class SimulCreator: public ListElement {
public:
  virtual SimulObject* create(SimulObjectType t) {return NULL;}
  virtual SimulObjectType type(char *ident) {return _UNDEF;}
};

class Simul: public SimulObject {
private: 
public:
  update();
  ~update();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _SIMUL : SimulObject::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() { Error("Simul can't be cloned!"); return NULL; }
  int snap();
  int replay();
};

#endif






