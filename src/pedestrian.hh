#ifndef _PEDESTRIAN_HH_
#define _PEDESTRIAN_HH_

#define PedActive      1
#define PedPeriodic    2
#define PedRandomize   4
#define PedSelected    8
#define PedVisible   128

#include <stdio.h>
#include <stdlib.h>
#include "list.hh"
#include "vector.hh"
#include "simul.hh"
#include "graphic.hh"

class Network;
class Gate;
class Street;
class Corridor;
class Floor;
//class Ground;
class Marker;

template<class STRUKTUR> class ARR {
private:
  STRUKTUR *a;
  int NN, MM;
public:
  ARR() {a = NULL; NN = MM = 0;}
  ~ARR() {
    delete [] a;
  }
  dim(int n, int m) {
    if (a) delete [] a;
    NN = n; MM = m;
    a = new STRUKTUR[NN*MM];
  }
  STRUKTUR& operator () (int i) {return a[i];}
  STRUKTUR& operator () (int i, int j) {return a[i*MM+j];}
  //  const STRUKTUR* operator(STRUKTUR*)() {return a;}
  int N() {return NN;}
  int M() {return MM;}
};


struct WalkerFlag {
  unsigned int active : 1;
  unsigned int periodic : 1;
  unsigned int xray : 1;
  unsigned int ellip : 1;
  unsigned int sum : 1;
  unsigned int : (sizeof(int)*8-7);
  unsigned int reset : 1;
  unsigned int visible : 1;
  operator int() {return (int)(*this);}
};

/* extern "C" {
  int read(int, char*, int);
  int write(int, char*, int);
  } */

enum WalkerRecordBlockType { WRBmisc, WRBdata, WRBdata2, WRBdata3 };
enum WlakerRecordBlockControl {WRB_START = 0, /* weitere ,*/ WRB_END = 31};

class WalkerRecordBlock {
public: 
  WalkerRecordBlockType type : 2;
  unsigned int continued : 1;
  union {
    struct {
      unsigned int control : 5; 
      unsigned int version : 32;  // 4 bytes 
      fixed4 time;                // 4 bytes 
      unsigned int number  : 32;       // 4 bytes   
    } misc;
    struct {
      unsigned int clan : 5;
      fixed4 x, y;   // 8 bytes 
      fixed4 vx, vy; // 8 bytes Betrag der Geschwindigkeit
      fixed4 v0;     // 4 bytes Betrag der Wunschgeschw.
      unsigned int from_gate : 4; 
      unsigned int to_gate : 4;
    } data;
    struct {
      unsigned int dummy : 5;
      fixed4 x0, y0, tneed, lneed, width; // 5*4 bytes 
      unsigned int b:8;
    } data2;
    struct {
      unsigned int dummy : 5;
      fixed4 c[5];   // 5*4 bytes Beschleunigung
      unsigned int d:8;
    } data3;
  };

  int write(int f) { return ::write(f, (char*)this, sizeof *this);} 
  int read(int f) { return ::read(f, (char*)this, sizeof *this);} 
};

class Walker: public Actor /* public SimulObject,  public Graphical */ {
friend main();
protected:
  double width;
  int clan;
  WalkerFlag *flag;
  int color;
  char draw_flag;
public:
  double *v0;
  Vector **Xtail;
  int Ntail;
  int *itail;
  int nr;
  Status<Vector> *X, *X0, *Xp, *V, *E0;
  Walker();
  void init();
  void clock();
  ~Walker();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _WALKER : Actor::objecttype(--i);
  } 
  unsigned int queryiosize() {return sizeof(struct WalkerRecordBlock);}
  int write(int);
  int mwrite(int);
  int mread(int,int);
// MOTIF
  void draw(class SzeneFenster*, Drawable);
// UNIRAS
  void hardcopy(class SceneryPlot*);
  void trace(class SceneryPlot*);
};


class Pedestrian: public Walker {
protected: 
  Floor *floor;
  double timer, expect;
  double rmax, lam, mu, pi, sigma, delta, gam, weightShift;
  int grpsize;
  int nr, funct;
  unsigned long int id;
  int draw_j, draw_k, draw_x, draw_y;
  double *alpha;
  Street *street;
  List attractions;
  List grounds;
  List markers;
  // individual
  Gate **next;
  Pointer<Gate> *dest, *orig;
  double *vmax;
  double *delay, *T;
  double *Tneeded, *Dneeded, *Texpected; // , Dexpected;
  double *Ttotal, *Ttotex, *Dtotal; //, Dtotex;
  // Friends
friend class Gate;
public:
  double *tau;
  Data *eta;
  Data *zeta; // total
  Data *zeta_ped, *zeta_build, *zeta_attr, *zeta_group;
  Data *negentropy; // negentropy 
  Data *xidata;
  //  Data *Y; 0 = Y1 efficiency, 1 = Y3 comfort, 2 = Y4 group, 3 = Y5 segr. 
  ARR<Data> Y;
  int NY;
  // double **P, **Pac; 
  ARR<double> P, Pac;
  int Nclans;
  Pedestrian();
  Pedestrian(Pedestrian*);
  ~Pedestrian();
  operator= (Pedestrian&);
  // Functions 
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _PEDESTRIAN : Walker::objecttype(--i);
  } 
  void init(); // aus SimulObject
  int ref(SimulObject *obj, SimulObjectType t);
  //SimulObject* clone();
  List gates;
  virtual void reset(int z=-1);
  void variables(int, double*);
  int update();
  Vector F(Vector& Xb, int w=-1);
  double U(Vector& Xb, int w=-1);
  // WWs
  Vector Fped(int w, double*, int*, double*);
  Vector Fped(int w, double*, int*);
  int write(int);
  int mwrite(int);
  int mread(int,int);
};

#include "evolution.hh"

class EvalPed: public Pedestrian {
protected: 
  Chromosome *s; // pro Pedest ein Chromosom
  Data *fitness;
  int *last_gate;
  Genotype *S;
  struct {
    double T, Dt;
    double best, average;
    int epoch;
    int mu;
  } evol;
  FILE *stream, *stream2;
public: 
  EvalPed();
  ~EvalPed();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _EVALPEDEST : Pedestrian::objecttype(--i);
  } 
  void init(); // aus SimulObject
  void reset(int z=-1);
  int update();
  Gate* select(Gate*, Vector&, double, double*, Vector*, int*);
  // MOTIF
  // void chart(class EvoFenster);
};

class SimulCreatorPedestrian: public SimulCreator {
public:
  SimulObject* create(SimulObjectType t);
  SimulObjectType type(char *ident);
};

#endif









