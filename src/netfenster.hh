#ifndef _NETFENSTER_HH_
#define _NETFENSTER_HH_
class Network;
class Net;
#include "fenster.hh"

enum ScaleStyle {UNDEF, Temperature, Rainbow, Grayscale, Geography};

class NetFarbScale: public ColorTableClass {
private:
  int Nscale;
  unsigned long *scalevector;
  unsigned long blackpix, graypix, whitepix;
  double smin, smax;
public:
  NetFarbScale(Display*, Window, int);
  ~NetFarbScale();
//  FarbPalette(Display *dis, int np);
//  FarbPalette(Display *dis, int np, int ns, double min, double max);
  unsigned long int black() {return blackpix;}
  unsigned long int gray() {return graypix;}
  unsigned long int white() {return whitepix;}
  void limits(double, double, ScaleStyle);
  void limits(double min, double max) {limits(min, max, UNDEF);}
  void limits(int min, int max, ScaleStyle st) {
    limits(double(min), double(max), st);
  }
  void limits(int min, int max) {limits(min, max, UNDEF);}
  unsigned long scale(double);
  unsigned long scale(int x) {return scale(double(x));}
  void drawscale(Drawable, GC, int, int, unsigned int, unsigned int);
};

class NetFenster: public Fenster {
public:
  Widget area, verti, hori, message;
  Pixmap vis;
  NetFarbScale *farbe;
  int Ngc;
  GC *gc;
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
  }  old;
  double valMin, valMax;
  bool scrpermit;
  char flag, screen_flag, draw_flag;
  //Network *network;
  Network *network;
  //public:
  NetFenster(Display*, Network*);
  ~NetFenster();
  void draw();
  Boolean scrollpermit() { return scrpermit; }
  //Umrechnungs Funktionen 
  int xx(double X) {return (int)(pixpm * (X-range.x1)) - px;}
  int yy(double Y) {return (int)(pixpm * (Y-range.y1)) - py;}
  int xpim(double X) {return (int)(pixpm * (X-range.x1));}
  int ypim(double Y) {return (int)(pixpm * (Y-range.y1));}
};
#endif







