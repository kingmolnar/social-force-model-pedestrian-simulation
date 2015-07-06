#ifndef _PLOT_HH_
#define _PLOT_HH_
#include "list.hh"
#include "param.hh"
#include "simul.hh"

#define MAXDEVICES 4
#define MAXTYPES 4

/* #define PlotCOLOUR 1 #define PlotRANGE  2 #define PlotSYMBOL 4
   #define PlotSHADOW 8 
   #define PLOTMAXVALUES 16
   
   #define PLOT_TOP_VIEW 1
   #define PLOT_DENSITY_VIEW 2
   #define PLOT_FORCE_ISOLINES 3
   #define PLOT_TRACES_OF_PED 4
   #define PLOT_CANCEL 99
   */

class Plot: public SimulObject {
friend class SceneryPlot;
friend class ConturPlot;
protected:
  float width_mm, height_mm;
  int width_pix, height_pix;
  unsigned int plotisopen : 1;
  unsigned int plotisrunning : 1;
  int strobo_rate, rcounter;
  char *comm;
  char *shell;
public:
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _PLOT : SimulObject::objecttype(--i);
  } 
  int sty;
  double arrowlength, arrowwidth, arrowratio;
  double framewidth;
  double linewidth;
  double duration, interval;
  double pedvmax, pedwidth;
  SimulObject* clone() { Error("Simul can't be cloned!"); return NULL; } 
//  SimulObject* simobj;
  void open();
  void close();
  virtual void plot() = 0;
  bool running() {return plotisrunning;}
 void panelActivate(Widget a, Widget b, char *s);
};

class Floor;
class Ground;
class Movie;
class Street;
class Pedestrian;

class SceneryPlot: public Plot {
  Floor *floor;
  List streets;
  Ground *ground;
  Movie *movie;
  double xmin, xmax, ymin, ymax, xoff, yoff, xsize, ysize;
public:
  SceneryPlot();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _SCENERY_PLOT : Plot::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  void plot();
  union {
    struct {
      unsigned int color : 1; // 0
      unsigned int rangefull : 1; // 1
      unsigned int backgroundoff : 1; // 2 
      unsigned int trace : 1; // 3
      unsigned int obstaclestyle : 3; // 4 5 6
      unsigned int : 2; // 7 8
      unsigned int walkerstyle : 4; // 9 10 11 12
      unsigned int axis : 1; // 13 
      unsigned int axisstyle : 3; // 14 15 16
      unsigned int : sizeof(int)*8-16;
    } flag;
    int intflag;
  };
  int colorindex[8];
//  void panelActivate(Widget a, Widget b, char *s) {panel(3, a, b, s);}
}; 

class ConturPlot: public Plot {
  Floor *floor;
  Street *street;
  Ground *ground;
  List pedestrians;
  float xmin, xmax, ymin, ymax, zmin, zmax, *z, zz;
  double xoff, yoff, xsize, ysize;
  struct {
    unsigned int color : 1;
    unsigned int range : 1;
  } flag;  
public:
  ConturPlot();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _CONTUR_PLOT : Plot::objecttype(--i);
  } 
  void init();
  int ref(SimulObject*, SimulObjectType);
  void plot();
}; 

// class GateStatPlot: public Plot {
// public:
//   GateStatPlot(Parameter*, SimulObject*);
//   void plot();
// }; 

#endif









