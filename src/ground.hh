#ifndef _GROUND_HH_
#define _GROUND_HH_

#include <stdio.h>
#include "list.hh"
#include "vector.hh"
#include "simul.hh"

typedef unsigned long XID;
typedef XID Drawable;

class Ground;
class Floor;
struct RingElement {
  Vector *X;
  double *a;
  int N;
};

class Marker: public SimulObject {
private:
  Floor *floor;
  Ground *ground;
//  Walker *walker;
  IntVector *I;
  Vector *X;
  double *val;
  int number;
  double sigma, beta;
  struct RingElement *array;
  int iring, nring, Nmax;
  union {
    struct {
      unsigned int faktor : 3;
      unsigned int : 1;
    } flag;
    int intflag;
  };
public:
  Marker();
  ~Marker();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _MARKER : SimulObject::objecttype(--i);
  } 
  void init(); // aus SimulObject
  int ref(SimulObject *obj, SimulObjectType t);
  void add(Vector);
  void add(Vector, double);
  void sub(Vector);
  void sub(Vector, double);
  void stamp(Vector, double);
  void stamp(Vector, Vector, double);
  void update();
  int N() {return number;}
};

class Grid {
protected:
  Vector Xmin, Xmax;
  int Npoints;
public:
  virtual int indx(IntVector Ipos) = 0;
  virtual int indx(int ix, int iy) = 0;
  virtual int indx(Vector) = 0;
  virtual int inside(IntVector Ipos, int*) = 0;
  virtual int inside(int, int, int*) = 0;
  virtual int inside(Vector, int*) = 0;
  int N() {return Npoints;}
}; 

class QGrid: public Grid {
protected:
  Vector Dx;
  IntVector N; // (nx, ny ...)
public:
  QGrid(Vector&, Vector& , Vector&);
  ~QGrid() {}
  int indx(IntVector Ipos) {return N.y*Ipos.x+Ipos.y;}
  int indx(int ix, int iy) {return N.y*ix+iy;}
  int indx(Vector);
  int inside(IntVector Ipos, int* x);
  int inside(int, int, int*);
  int inside(Vector, int*);
};

class Floor;

class Ground: public SimulObject {
friend class Marker;
private:
  Floor *floor;
  Grid *grid;
  double *g;
  Vector Xmin, Xmax, Dx;
  double gmin, gmax;
  double dec, tau, beta;
  Vector E;
  double T;
  int timeSkip;
  union {
    struct {
      unsigned int autoscale : 1;
    } flag;
    int intflag;
  };
  char drawflag;
  //  char *title;
  //  char *fileName;
  //  FILE *fPointer;
public:
  Ground();
  ~Ground();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _GROUND : SimulObject::objecttype(--i);
  } 
  operator =  (double z) {int NN=grid->N(); for (int i=0; i<NN; i++) g[i]=z;}
  operator += (double z) {int NN=grid->N(); for (int i=0; i<NN; i++) g[i]+=z;}
  operator -= (double z) {int NN=grid->N(); for (int i=0; i<NN; i++) g[i]-=z;}
  operator *= (double z) {int NN=grid->N(); for (int i=0; i<NN; i++) g[i]*=z;}
  operator /= (double z) {int NN=grid->N(); for (int i=0; i<NN; i++) g[i]/=z;}
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  double U(Vector&);
  Vector F(Vector&);
  void decay();
  void adjust();
  //int read(FILE*), update();
  int mread(int, int);
  int mwrite(int);
  void draw(class SzeneFenster*, Drawable);
  void hardcopy(class SceneryPlot*);
};

class SimulCreatorGround: public SimulCreator {
public:
  SimulObject* create(SimulObjectType t);
  SimulObjectType type(char *ident);
};
#endif
