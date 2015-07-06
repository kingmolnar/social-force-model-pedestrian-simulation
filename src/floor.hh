#ifndef _FLOOR_HH_
#define _FLOOR_HH_
#include <stdio.h>

/* Object Floor */
/* global definitions */

// MOTIF
typedef unsigned long XID;
typedef XID Drawable;
class Fenster;
class SzeneFenster;

#include "vector.hh"
#include "simul.hh" 
#include "param.hh"
#include "arrlst.hh"

class Network; 
class Floor: public WithLogfile { // SimulObject {
protected: 
public: 
  char *build;
  int Np, Na, Ns, Nn, Ngat;
  int snapshot, trace;
  int initialize;
  int *tag;
  char datafile[64], titlefile[64];
  char flag;
  int Nclans, Nparts;  // Anzahl der partiellen Verteilungen
  int Nvars;
  int progM;
  int progMIN;
  arrlst<Data*> *journ;
  DataPointer *performance;// (part.) Statistik der Eigenschaften d. Korridors
  PrognosisPointer *prog;
  FILE *prffp;
  Vector Xmin, Xmax, Dx;
  double rhomin, rhomax;
  double beta, Delta, Tau, Dt, T, tresh, weight;
  double sigma_B, epsilon_B, epsilon_P;
  double vmin, vmax;
  // objects
  List pedestrians;
  List streets;
  List gates;
  Network *network; 
  List grounds, markers;
  List attractions;
  //List obstacles;
  //List nodess;
  // functions
  Floor();
  ~Floor();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _FLOOR : WithLogfile::objecttype(--i);
  } 
  void init(); // neu statt SimulObject
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {};
  void calc();
  int update();
  Data* journey(int cl, int part);
  Data* journey(int cl, double v0);
  // Data* journey(int cl, double v0, double td, double tt, double ed, 
  // double et); 
  // Data* journey(int cl, double v0, double eta, double zeta,
  // double neg, double xi);
  Data* journey(int cl, double v0, double *x, int nvars);
  void log();
  // #ifdef MOTIF void draw(Fenster*, Drawable); #endif
  // (*time_draw)(), (*hardcopy)();_ROUTE_HH_
  //int read(FILE*), write(FILE*);

  int mread(int, int);
  int mwrite(int);
  
  //read_data(FILE), write_data(FILE);
  //int replay(FILE);
  //  FILE *LogFile;
};

#define FloorReplay 8
#define FloorRecord 16
#define FloorTitle  32

/* class Labyrinth: public Floor {
protected: 
  struct RouteElement {
    double distance;
    double velocity;
    class Gate* next;
    class Street* street;
  }
  RouteElement *route;
  int ngates;
  RouteElement *r(int i, int j) {return route+i*ngates+j;} 
public: 
  Labyrinth();
  ~Labyrinth();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _LABYRINTH : Floor::objecttype(--i);
  } 
  void init(); // neu statt SimulObject
  // int ref(SimulObject *obj, SimulObjectType t);
  int update();
  // Wegweiser
  void gone(class Pedestrian*);
}; */

class SimulCreatorFloor: public SimulCreator {
public:
  SimulObject* create(SimulObjectType t);
  SimulObjectType type(char *ident);
};
#endif









