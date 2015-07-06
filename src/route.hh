#ifndef _ROUTE_HH_
#define _ROUTE_HH_

#include <stdio.h>
#include "list.hh"
#include "vector.hh"
//#include "param.hh"
#include "graphic.hh"
#include "simul.hh"
//#include "network.hh"

class Pedestrian;
class Street;
class Mall;


/* class Vertex: public SimulObject, public VertexClass {
   private:
   int funct, color;
   VertexClass vertex;
   public:
   List environments;
  Vertex();
  void init();
  int refup(SimulObject*);
  Vector dest(Vector);
  }; 
  */

class Gate: public SimulObject {
private:
  static Mall *mall;
  int funct;
  //  unsigned long int id, streetId;
public:
  Street *street[2];
  Gate();
  //  ~Gate();
  virtual void init();
  virtual int refup(SimulObject*);
  virtual int refdown(SimulObject*);
  Vector P, Q;
  int nr;
  virtual int write(FILE*);
  virtual int simul(Pedestrian*);
  virtual Vector dest(Vector);
  virtual Vector dest(Gate *gat);
  //operator Index() { return i;}
  void draw(Fenster*, Drawable);
};

/* class Alternative: public Gate {
private: 
  List gates;
public:
  Alternative();
  //  ~Alternative();
  void init();
  int refup(SimulObject*);
  int refdown(SimulObject*);
  //int write(FILE*);
  ///int simul(Pedestrian*);
  Vector dest(Vector);
  Vector dest(Gate *gat);
  //operator Index() { return i;}
  //  void draw(Fenster*, Drawable);
};  


class Source: public Gate {
public:
  double rate;
  double t[2];
  Sink *sink;
  List pedestrians;
  Source();
  // ~Source() {}
  void init();
  int refup(SimulObject*); 
  int refdown(SimulObject*);
};

class Sink: public Gate {
public:
  struct {
    unsigned int restart : 1;
    unsigned int xxxx : 1;
  };
  Source *source;
  Sink();
  // ~Sink() {}
  void init();
  int refup(SimulObject*); 
  int refdown(SimulObject*);  
};
  
class Intersection: public Gate {
public:
  double delay[2];
  bool queue;
  int capacity;
};

*/


class Network: public SimulObject {
}
  
#endif













