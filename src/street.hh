#ifndef _STREET_HH_
#define _STREET_HH_

#include <stdio.h>
#include "vector.hh"
#include "simul.hh"
#include "graphic.hh"

#ifndef NULL
#define NULL ((void*)0)
#endif

class Street;
class Floor;
class Pedestrian;
class Walker;

class Obstacle: public ListElement {
friend class Wall;
friend class Polygon;
friend class CWall;
friend class Circle;
public:
  Vector P;
  double beta, sigma, rmax; // epsilon
  //float *u;
  //unsigned int N;
  int color;  
  char draw_flag;
  bool avoid;
  Obstacle() {};
  //  virtual ~Obstacle();
  virtual Vector dest(Vector) = 0;
  virtual Vector F(Vector);
  virtual double U(Vector);
  virtual int intersect(Obstacle*) = 0, intersect(Vector, Vector) = 0;
  virtual void print(FILE*);
  // MOTIF
  virtual void draw(class SzeneFenster*, Drawable) = 0;
  // UNIRAS
  virtual void hardcopy(class SceneryPlot*) = 0;
};

class Polygon: public SimulObject {
private:
  Street *street;
  int nwall;
public:
  Polygon();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _POLYGON : SimulObject::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {};
};

class Wall: public Obstacle {
friend class Polygon;
protected: 
  Vector Ep; //, Es;
  double a; //, b, c, ap, bp, cp, l;
  // double xmin, ymin, xmax, ymax;
  int N;
  Vector *PP;
  Vector *EEp;
  double *aa;
public:
  Wall() {PP = 0; EEp = 0; aa = 0; N = 0; }
  ~Wall() {if (PP) delete[] PP; if (EEp) delete[] EEp; if (aa) delete[] aa;}
  Vector dest(Vector);
  Vector dest(Vector, int);
  int intersect(Obstacle*);
  int intersect(Vector, Vector);
  int intersect(Vector, Vector, int);
  void print(FILE*);
  // MOTIF
  void draw(class SzeneFenster*, Drawable);
  // UNIRAS
  void hardcopy(class SceneryPlot*);
};


class Circle: public SimulObject {
private:
  Street *street;
public:
  Circle();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _CIRCLE : SimulObject::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {};
};

class CWall: public Obstacle {
friend class Circle;
protected:
  double radius;
public:
  //  CWall(Vector, double, double, double, double, int);
  Vector dest(Vector);
  //  virtual  Vector F(Vector);  virtual  double U(Vector);
  int intersect(Obstacle*), intersect(Vector, Vector);
  void print(FILE*);
  // MOTIF
  void draw(class SzeneFenster*, Drawable);
  // UNIRAS
  void hardcopy(class SceneryPlot*);
};

class Decoration: public SimulObject {
private:
  enum {DecoNULL, DecoPOLYGON, DecoCIRCLE, DecoTEXT} type;
  int N;
  float *x, *y;
  double radius;
  int color, framecolor;
  double framewidth, textheight;
  char *text;
public:
  Decoration();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _DECORATION : SimulObject::objecttype(--i);
  } 
  void init();
  SimulObject* clone() {}
  //int ref(SimulObject *obj, SimulObjectType t);
  // MOTIF
  void draw(class SzeneFenster*, Drawable);
  //UNIRAS
  void hardcopy(class SceneryPlot*);
};

class Street: public SimulObject /* , public Graphical */ {
protected: 
  Floor *floor;
  float *u;
  //  int Nnodes, Nknots, Nobs, Ncircles, Nwalls, Natts, Npeds;
public: 
  double beta, sigma, rmax;
  int forcefct;
  Data E; // efficency
  Data V; // stress
  Data S; // negentropy
  List adjacents;
  List obstacles;
  List decorations;
  List heap;
  List attractions;
  List locations;
  List gates;
  Vector Xmin, Xmax, Dx;
  Vector F(Vector);
  Vector F(Vector, Vector, double);
  double U(Vector);  
  Street();
  ~Street();  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _STREET : SimulObject::objecttype(--i);
  } 
  List pedestrians;
  List vertices;
  //  List gates;
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {};
  //  int read(FILE*), write(FILE*);
  void print(FILE*);
  void draw(class SzeneFenster*, Drawable); // MOTIF
  void hardcopy(class SceneryPlot*); // UNIRAS
};

class Grid;
class Data;

class Corridor: public Street {
private:
  //int data_rate; // es werden auch Daten ueber mehrere Zeitschritte gesammelt
  double mbeta, msigma, mrmax;
  Vector *X;
  double *val;
  int number;
  FILE *stream;
public: 
  Grid *grid;
  double *part_rho; // Flaechenverteilung der (partiellen) Dichte
  double *faktor;    // Skalierungsfaktor fuer Verteilungsfunktion
                     // fast ueberall 1.0, auf gesperrten Flaechen 
                     // gleich 0.0 und an den Raendern groesser 1.0
  int Nparts;        // Anzahl der partiellen Verteilungen 
  double neg_entropy; // Negentropie
  DataPointer *performance;// (part.) Statistik der Eigenschaften d. Korridors
  Corridor();
  ~Corridor();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _CORRIDOR : Street::objecttype(--i);
  } 
  void init();
  // ref() wie street;
  const int Nvariables = 4;
  void data(int cl, Vector V, double v_momentan, double v0, 
	    double f_gesamt, double r_naehe_zu_anderen); 
  void prologue();
  void epilogue();
  void report(FILE* s=0);  
};

enum LocationShape {DOT, LINE, CIRCLE, RECTANGLE};

class Location: public WithLogfile { // public SimulObject {
protected:
public:   
  Street **street;
  int nr;
  Vector P;
  int color;  
  char draw_flag;
  LocationShape shp;
//  virtual Vector position() {return P;}
  virtual Vector dest(Vector& X) {return P-X;}
  virtual bool passed(Vector&, Vector&);
  Location();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _LOCATION : SimulObject::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {return NULL;}
  virtual Vector rndpos() { return P;}
  virtual void print(FILE*);
};

class Gate: public Location {
protected: 
  Floor *floor;
  double radius;
  int nr;
  int funct;
  double dec;
public: 
  struct {
    Vector Vel;
    int n;
    void clear() {Vel = Vector(0.0,0.0); n = 0;}
    int N() {return n;}
    void data(const Vector& X) {Vel += X; n++;}
    Vector mean() { return n? Vel/n: Vector(0.0,0.0);}
  } vel;
  double vmax;
  Vector Q;
  Gate();
  //  ~Gate();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _GATE : Location::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  SimulObject* clone() {return 0;}
  void print(FILE*);
  virtual Vector rndpos();
  int simul(Pedestrian*);
  Vector dest(Vector&);
  bool passed(Vector&, Vector&);
  void velocity(Vector&, Vector&);
  Vector velocity() {return vel.Vel;}
  virtual int Nalt() {return 1;}
  virtual Gate& operator[](int i) {return *this;}
  //  Street *link(Gate*);
  Street *otherstreet(Street *str) {
    return !street ? 0 : (str==street[0] ? street[1] : street[0]);
  }
  void log();
  void draw(class SzeneFenster*, Drawable); // MOTIF
  void hardcopy(class SceneryPlot*); // UNIRAS
};


class MultipleGate: public Gate {
protected: 
  double xsi, lambda;
public: 
  List gates;
  MultipleGate();
  ~MultipleGate();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _MULTIPLEGATE : Gate::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  int Nalt() {return gates.N();}
  Gate& operator[](int i);
  Vector dest(Vector&);
  bool passed(Vector& X, Vector& Y);
  void draw(class SzeneFenster*, Drawable); // MOTIF
  void hardcopy(class SceneryPlot*); // UNIRAS
};  
 
class Door: public Gate {
protected: 
  double roffset;
  struct {
    unsigned int punkt : 1;
    unsigned int zero  : 1;
  } flag;
  Vector Pp, Qp;
public: 
  Door();
  ~Door();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _DOOR : Gate::objecttype(--i);
  } 
  void init();
  Vector dest(Vector&);
  Vector rndpos();
};  

class SimulCreatorStreet: public SimulCreator {
public:
  SimulObject* create(SimulObjectType t);
  SimulObjectType type(char *ident);
};
#endif
