#ifndef _GRAPHIC_HH_
#define _GRAPHIC_HH_

class Fenster;
class myplot;
typedef unsigned long XID;
typedef XID Drawable;

class Graphical {
public:
  int color;
  char draw_flag;
  /*   virtual void draw(Fenster *f, Drawable drw) {}
       virtual void draw(Fenster *f, Drawable drw, int style) {draw(f, drw);}
       virtual void hardcopy(myplot *pl) {}
       virtual void hardcopy(myplot *pl, int style) {hardcopy(pl);}
       */
};
#endif
