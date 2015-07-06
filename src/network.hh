#ifndef _NETWORK_HH_
#define _NETWORK_HH_
#include "vector.hh"
#include "graphic.hh"
#include "random.hh"
#include <iostream.h>
typedef int Index;

#define UNEND 1.0e20


class VertexClass: public virtual Graphical {
private:
  Index i;
public:
  int weight;
  Vector P;
  Index index() {return i;}
  Index index(Index ind) {i = ind; return i;}
  operator Index() {return i;}
  virtual Vector dest(Vector A) {return P-A;}
  virtual Vector dest(VertexClass *v) {return P - v->P;}
  void draw(Fenster*, Drawable);
};

typedef VertexClass* VertexPointer;

class EdgeClass: public virtual Graphical {
public:
  int frequency;
  VertexPointer *vert; // mostly two, but may be more
  int Nvert;
  // Properties 
  double quality, effort, voffset;
  double vEff, timeEff, lengthEff;
  EdgeClass(): frequency(0), Nvert(0) {}
  void draw(Fenster*, Drawable);
};

typedef EdgeClass* EdgePointer;

class NetMatrixElement {
friend class Net;
public:
  NetMatrixElement() {value = UNEND; weight = 1;}
  virtual ~NetMatrixElement() {}
  EdgeClass *edge;
  VertexClass *next;
  double value;
  int weight, z;
  /*   virtual bool valid() {return value < UNEND;}
       virtual bool greater(NetMatrixElement *a, NetMatrixElement *b) {
       return (a->value+b->value) < value;
       }
       virtual void link(NetMatrixElement *a, NetMatrixElement *b) {
       next = a->next;
       //    egde = NULL;
       value = a->value + b->value;
       } */
};


class Net {
private:
protected:
  int T; 
  RanSequence *rs;
  double treshold;
public:
  NetMatrixElement *matrix;
  int N, Ne;
  VertexPointer *Vert;
  EdgePointer *Edg;
  Net(); 
  //  Net(List&, List&);
  // Net(List&, List&, unsigned long);
  //  Net(int, VertexClass**, int, EdgeClass**);
  //  Net(int, VertexClass**, int, EdgeClass**, unsigned long);
  ~Net() {};
  //  virtual NetMatrixElement *M(Index i, Index j) {return matrix+i*N+j;}
  NetMatrixElement *M(Index i, Index j) {return matrix+i*N+j;}
  NetMatrixElement *M(VertexClass *i, VertexClass *j) {
    return matrix+i->index()*N+j->index();
  }
//  NetMatrixElement *M(Pointer<VertexClass>& i, Pointer<VertexClass>& j) {
//    return matrix+i->index()*N+j->index();
//  }
  
  double value(Index i, Index j) {return M(i, j)->value;}
  VertexClass *next(Index i, Index j) {return M(i, j)->next;}
  EdgeClass *edge(Index i, Index j) {return M(i, j)->edge;}

  virtual bool valid(Index a, Index b) {return M(a,b)->value < UNEND;}
  virtual double length(Index a, Index b) { return M(a,b)->value; } 
  virtual void link(Index a, Index b, Index c) { 
    M(a,c)->next = M(a,b)->next;
    M(a,c)->value = M(a,b)->value + M(b,c)->value;
    M(a,c)->edge = NULL; // direkte Verbindung geht verloren;
  }
  
  //  void fill(VertexClass**, EdgeClass**);
  virtual void fill();
//  virtual void fill(NetList&, NetList&);
  int warshall();
  int count(int*, double*);
  int count() {return count(NULL, NULL);}
  void write();
};


/* 
class Network: public Net {
public:
  Vector Xmin, Xmax;
  List vertices;
  List edges;
  Network(NetList&, NetList&, unsigned long);
  ~Network();
  void draw(class NetFenster*, Drawable);
  void harcopy(Plot*);
};
*/

/*
class Edge: public SimulObject, public EdgeClass /* , public Graphical */ {
public:
  Edge();
  List vertices;
  void init();
  int refdown(SimulObject*);
};

class RouteElement: public NetMatrixElement {
public:
  //  Street *street;
  //  Gate *nextg;
  /* Statistics */
  double quality, effort, voffset;
  double vEff, timeEff, lengthEff;  
  double *TrelMean, *TrelVar, TrelM, TrelV;
  double *DrelMean, *DrelVar, DrelM, DrelV;
  int *vcount; 
  //int *classCount;
};

 
class Route: public NetMatrix{
private:
  int N;
  RouteElement *re;
public:
  Route(int n, int nv, int nclan, int nrank);
  ~Route();
  RouteElement *M(Index k, Index l) {return re + k * N + l;}
};

class Network: public Net {
public:
  Vector Xmin, Xmax;
  List vertices;
  List edges;
  Network(List&, List&, unsigned long);
  ~Network();
  void draw(class NetFenster*, Drawable);
  void harcopy(Plot*);
};

class Network: public SimulObject, public Net {
public:
  int N, numberv, numberClan, numberRank;
  // Gate **gatVect;
  // Route *route;
  Mall *mall;
  Vector Xmin, Xmax;
  List edges;
  List vertices;
  Network();
  ~Network();
  //  int netsearch();
  int feedup();
  Gate* next(Index i, Index j) {return (Gate*)Net::next(i,j);}
  //Street* street(Index i, Index j);
  //  EnvironmnetClass *environment(Index i, Index j);
  int clear();
  int read(FILE*), write(FILE*);
  void init();
  int refdown(SimulObject*);
  
  // int warshall();  
  // List& routelist(List&);
  // List& routelist(Pointer<VertexClass>);
  void draw(class NetFenster*, Drawable);
  void harcopy(Plot*);
};
 
*/


#endif







