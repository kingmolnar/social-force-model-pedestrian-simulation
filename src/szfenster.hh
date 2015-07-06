#ifndef _SZFENSTER_HH_
#define _SZFENSTER_HH_

class Floor;
class Network;
class Net;
class Ground;
class Movie;

#include "fenster.hh"

class GraphicSymbol {
public: 
  SzeneFenster *f;
};

class GraphicCircle: public GraphicSymbol {
private: 
  unsigned int w, h;
public: 
  GraphicCricle(SzeneFenster *ff)  {f = ff;}
  void scale(double);
  void draw(Drawable drw, Vector X, double v, int c);
};

class GraphicArrow: public GraphicSymbol {
private: 
  double length, width, ratio, vmax;
public: 
  GraphicArrow(SzeneFenster *ff)  {f = ff;}
  void scale(double l, double w, double r, double m) {
    length = l; width = w; ratio = r; vmax = m;
  }
  void draw(Drawable drw, Vector X, Vector V, int c);
};


#define NGC 2
class SzeneFenster: public Fenster {
// friend Walker::draw(SzeneFenster*, Drawable);
// friend Wall::draw(SzeneFenster*, Drawable);
// friend Circle::draw(SzeneFenster*, Drawable);
// friend Street::draw(SzeneFenster*, Drawable);
public:
  struct Symbol {
    GraphicArrow *arrow;
    GraphicCircle *circle;
  } symbol;
  Widget area, verti, hori, message;
  Pixmap pim, vis, unvis;
  FarbPalette *farbe;
  char *farbenliste;
  GC gc[NGC];
  int px, py;
  int paramChanged;
  unsigned int pwidth, pheight, width, height;
  int pixpm, linewidth;
  struct {
    float x1, y1, x2, y2;
  } range;
  struct {
    unsigned int width, height;
    int px, py;
  } old;
  double valMin, valMax;
  int Ngc;
  bool scrpermit;
  double vmax, pedwidth;
  char flag, screen_flag, draw_flag;
  /*   struct Pedestrian *fellow;
       int fellow_nr, nr; */
  union {
    struct {
     unsigned int color  : 1; // 0 
     unsigned int obstac : 1; // 1
     unsigned int attrac : 1; // 2
     unsigned int walker : 3; // 3 4 5
     unsigned int trace  : 1; // 6
     unsigned int : 3;        // 7 8 9
     unsigned int ground : 1; // 10
     unsigned int noscrollbars : 1; // 11
     unsigned int : 4;        // 12 13 14 15 
    } szflag;
    int intszflag;
  };
  void *tag, *userData;
  Floor *floor;
  Ground *ground;
  Movie *movie;
  //  callback functions
  //public:
  //  SzeneFenster(Display*, Floor*);
  SzeneFenster();
  ~SzeneFenster();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _SCENERY_WINDOW : Fenster::objecttype(--i);
  } 
  void init();
  int ref(SimulObject *obj, SimulObjectType t);
  void draw();
  void redraw();
  Boolean scrollpermit() { return scrpermit; }
  void target(Floor*);
  /* Umrechnungs Funktionen */
  int xx(double X) {return (int)(pixpm * (X-range.x1)) - px;}
  int yy(double Y) {return (int)(pixpm * (Y-range.y1)) - py;}
  int xpim(double X) {return (int)(pixpm * (X-range.x1));}
  int ypim(double Y) {return (int)(pixpm * (Y-range.y1));}
};

class VideoFenster: public SzeneFenster {
  int timer;
  int delay;
public:
  VideoFenster();
  ~VideoFenster();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _SCENERY_WINDOW : Fenster::objecttype(--i);
  } 
  void init();
  void draw();
};

class ReplayFenster: public SzeneFenster {
  struct WBlock {
    Vector pos;
    Vector vel;
    double v0;
    int clan;
  } *W;
  int WN;
  int Nmemo;
  char *moviename;
  int movie;
  double T;
  double pedwidth;
public:
  ReplayFenster();
  ~ReplayFenster();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _REPLAY_WINDOW : SzeneFenster::objecttype(--i);
  }
  void init();
  void draw();
  int read();
};

#endif








