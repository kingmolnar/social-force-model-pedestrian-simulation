#ifndef _FENSTER_HH_
#define _FENSTER_HH_

#define FeBackgroundRestored 1
#define NCOLOR 10
#define NGC 2 

#include "simul.hh"

#include <MIT/X11/X.h>
#include <MIT/X11/Xlib.h>  
#include <Xt/Intrinsic.h>
#include <X11/Xm/Xm.h>
#undef NULL
#include <iostream.h>

extern "C" {
  Widget XmCreateDialogShell(Widget, char*, Arg*, int); 
  Widget XmCreateMainWindow(Widget, char*, Arg*, int);
  Widget XmCreateText(Widget, char*, Arg*, int);
  Widget XmCreateFrame(Widget, char*, Arg*, int);
  Widget XmCreateScrollBar(Widget, char*, Arg*, int);
  Widget XmCreateDrawingArea(Widget, char*, Arg*, int);
  
  void XmMainWindowSetAreas(Widget, Widget, Widget, Widget, Widget, Widget);
  XmTextSetString(Widget, char*);  
  char* XmTextGetString(Widget);
}
/* struct XmScrollBarCallbackStruct {
   int reason;
   XEvent *event;
   int value;
   int pixel;
   };
   
   struct XmDrawingAreaCallbackStruct {
   int     reason;
   XEvent  *event;
   Window  window;
   }; 
   */
externalref WidgetClass applicationShellWidgetClass;

#define XmVERTICAL	       1
#define XmHORIZONTAL           2


extern int FensterNummer;
#undef NULL
#include "param.hh"
#include "list.hh"

class ColorTableClass {
public:
  Colormap colormap;
  Display *dis;
  int screen;
  Visual *visual;
  unsigned long defpixel;
  ColorTableClass() {};
  ColorTableClass(Display*);
  virtual unsigned long black() {return BlackPixel(dis, screen);}
  virtual unsigned long white() {return WhitePixel(dis, screen);}
  virtual unsigned long pix(int i) {return defpixel;}
};

enum ScaleStyle {UNDEFscale, Temperature, Rainbow, Grayscale, Geography};

class FarbPalette: public ColorTableClass {
private:
  int Npix, Nscale;
  unsigned long *pixvector;
  unsigned long *scalevector;
  double smin, smax, sdx;
public:
  FarbPalette(Display *_d, int _npix = 8, int _nscale = 16);
  ~FarbPalette();
  unsigned long pix(int);
  unsigned long foreground();
  unsigned long background();
  unsigned long gray()  {return pixvector ? pixvector[-2] : defpixel;}
  void names(char *);
  void limits(double, double, ScaleStyle);
  void limits(double min, double max) {limits(min, max, UNDEFscale);}
  void limits(int min, int max, ScaleStyle st) {
    limits(double(min), double(max), st);
  }
  void limits(int min, int max) {limits(min, max, UNDEFscale);}
  unsigned long scale(double);
  unsigned long scale(int x) {return scale(double(x));}
  void drawscale(Drawable, GC, int, int, unsigned int, unsigned int);
};


class Fenster: public SimulObject /* WithParameter, public ListElement */ { 
public:
  int nr;
  Widget widget, controlPanel;
  char *controlPanelName;
  Display *dis;
  Window win;
  XWindowAttributes a;
  Font font;
  XFontStruct *fontstruct;
  bool scrpermit;
  // functions 
  Fenster();
  virtual ~Fenster() {cout << "good bye ...\n";}
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _WINDOW: SimulObject::objecttype(--i);
  } 
  Fenster *next;
  virtual void panel();
  virtual void panel(Widget *W, char *S) ;
  virtual void draw();
  virtual void redraw();
  virtual void input(XEvent*);
  bool scrollpermit() {return scrpermit;}
};

// Prototypes
extern "C++" {
  unsigned long MakeColor2(Display *dis, char *S);
  unsigned long MakeColor(Display *dis, char *S);
  unsigned long MakeRGB(Display*, Colormap, double, double, double);

  void Input(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *wD);
  void Exposure(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *cbs);
  void Resize(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *cbs);
  void Scroll(Widget w, Fenster* f, XmScrollBarCallbackStruct *cbs);
} 

#endif










