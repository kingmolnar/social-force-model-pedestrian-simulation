#define NTEST
#include "test.h"
#include "fenster.hh"
#ifndef NULL
#define NULL 0
#endif

//#include <Xt/Shell.h>
/* 
   #include <Xm/MainW.h>
   #include <Xm/ScrolledW.h>
   #include <Xm/ScrollBar.h>
   #include <Xm/Frame.h>
   #include <Xm/Form.h>
   #include <Xm/Text.h>
   #include <Xm/DrawingA.h>
   */

/* static */ int FensterNummer = 1;

// ---- FARBEN ---- 

unsigned long MakeColor2(Display *dis, char *S)
{
  XColor xcolor, xcolor_exact;
  int screen = XDefaultScreen(dis);
  Colormap cm =  XDefaultColormap(dis, screen); 
//  test(printf("makeclolor2 %s\n", S));
  XLookupColor(dis, cm, S, &xcolor, &xcolor_exact);  
  XAllocColor(dis, cm, &xcolor);
  return xcolor.pixel;
}

unsigned long MakeColor(Display *dis, char *S) 
{
  XColor xcolor, xcolor_exact;
  int screen;
  Colormap cm;
  screen = XDefaultScreen(dis);
  cm =  XDefaultColormap(dis, screen);
//  test(printf("makecolor %s\n", S));
  XParseColor(dis, cm, S, &xcolor);  
  XAllocColor(dis, cm, &xcolor);
  return xcolor.pixel;
}

unsigned long MakeRGB(Display *d, Colormap m , double r, double g, double b) {
  XColor xcolor;
  xcolor.red = short(65535.0*r);
  xcolor.green = short(65535.0*g);
  xcolor.blue = short(65535.0*b);
  xcolor.flags = DoRed | DoGreen | DoBlue;
 // test(printf("farbe %d,%d,%d\n", xcolor.red, xcolor.green, xcolor.blue));  
  XAllocColor(d, m, &xcolor);
  return xcolor.pixel;
}

ColorTableClass::ColorTableClass(Display *d) {
  dis = d; screen = XDefaultScreen(dis);
  visual =  DefaultVisual(dis, screen);
  colormap = XDefaultColormap(dis, screen);
  defpixel = black();
  printf("colortable constructor\n");
}

FarbPalette::FarbPalette(Display *d, int _npix, int _nscale) {
//  test (printf("construct FarbPalette ... "));
  dis = d;
  screen = XDefaultScreen(dis);
  colormap = XDefaultColormap(dis, screen);
  defpixel = BlackPixel(dis, screen);
  Npix = _npix;
  unsigned long plmask[1];
  pixvector = new unsigned long[Npix+2];
  while (1) {
    if (XAllocColorCells(dis, colormap, False, plmask, 0, pixvector, Npix+2))
      break;
    Npix--;
    if (Npix == 0) {}
  }
  //  pixvector[-2] = BlackPixel(dis, screen);
  //  pixvector[-1] = WhitePixel(dis, screen);
  /* Farben definieren */
  char *namen[] = { {"black"}, {"white"}, 
		    {"white"}, {"black"}, {"red"}, {"green"}, 
		    {"blue"}, {"cyan"}, {"magenta"}, {"gray"},
		    {"blue"}, {"cyan"}, {"magenta"}, {"gray"},
		    {"blue"}, {"cyan"}, {"magenta"}, {"gray"},
		    {"blue"}, {"cyan"}, {"magenta"}, {"gray"} };
  int nn = sizeof(namen)/sizeof(char*);
  XColor xcolor;
  for (int i=0; i<Npix+2 && i<nn; i++) {
    XParseColor(dis, colormap, namen[i], &xcolor);  
    xcolor.flags = DoRed | DoGreen | DoBlue;
    xcolor.pixel = pixvector[i];
    XStoreColor(dis, colormap, &xcolor);
  }  

  pixvector += 2;

  // scale
  if (_nscale%2==1) Nscale = _nscale;
  else Nscale = _nscale+1;
  scalevector = new unsigned long[Nscale+1];
  unsigned long plane_masks[1] ;
  while (1) {
    if ( XAllocColorCells (dis, colormap, False,
			   plane_masks, 0, scalevector, Nscale+1) )
      break ;
    Nscale -= 2;
    if (Nscale <= 0) {
      //      exit (0) ;
    }
  }
  //scalevector = NULL;
//  test(printf("done.\n"));
}
  
FarbPalette::~FarbPalette()
{
  if (pixvector) delete pixvector;
  if (scalevector) delete scalevector;
  Npix = Nscale = 0;
  XFreeColormap(dis, colormap);
}

unsigned long FarbPalette::pix(int i)
{
  if (pixvector && i>=0 && i<Npix)
    return pixvector[i];
  else return defpixel;
}

unsigned long FarbPalette::foreground()
{
  return pixvector ? pixvector[-2] : black(); 
}

unsigned long FarbPalette::background()
{
  return pixvector ? pixvector[-1] : white(); 
}


void FarbPalette::names(char *nam) {
  char *tok = strtok(nam," ");
  XColor xcolor;
  int i = -2;
  char *p;
  while (tok && *tok && i<Npix) {
    for (p=tok; p && *p; ++p) {
      switch (*p) {
      case '.':  case '_':  case '-':  case '|':  case '/':
	*p = ' ';
	break;
      case '%': case '$':
	*p = '#';
	break;
      }
    }
    //    cout << '`' << tok << '\'';
    XParseColor(dis, colormap, tok, &xcolor);
    xcolor.flags = DoRed | DoGreen | DoBlue;
    xcolor.pixel = pixvector[i];
    XStoreColor(dis, colormap, &xcolor);
    tok = strtok(NULL, " ");
    ++i;
  }  
}

void FarbPalette::limits(double min, double max, ScaleStyle st) {
  XColor xcolor;
  int i, n;
  double d, x;
  if (min<max) {
    smin = min;
    smax = max;
  }
  else {
    smin = max;
    smax = min;
  }
  if (Nscale) sdx = (smax - smin) / Nscale;
  test(printf("limits: "));
  switch (st) {
  case UNDEFscale:
    // nichts aendern!
    break;
  case Temperature: // blau-weiss-rot
  case Rainbow: // blau-gruen-gelb-rot
    announce("RAINBOW\n");
    double expo = 1.5;
    n = (Nscale+1)/2;
    d = 1.0/double(n-1);
    for (i=0; i<n; ++i) {      // RGB(x, 1.0, 1.0-x);
      x = pow(d*i, expo);      // XW=FLOAT(I-1)/99.
      xcolor.red = short(65535.0*x);
      xcolor.green = 65535;
      xcolor.blue = short(65535.0*(1.0-x));
      xcolor.flags = DoRed | DoGreen | DoBlue;
      xcolor.pixel = scalevector[i];
      XStoreColor(dis, colormap, &xcolor);
      //scalevector[i] = i;
    }                          // END DO
    for (i=n; i<Nscale; ++i) { // DO I=1,100
      x = d * (i-n);           // RGB(1.0, pow(1.0-x, expo), 0.0);
      xcolor.red = 65535;
      xcolor.green = short(65535.0*pow(1.0-x, expo));
      xcolor.blue = 0; // short(65535.0*(1.0-x));
      xcolor.flags = DoRed | DoGreen | DoBlue;
      xcolor.pixel = scalevector[i];
      XStoreColor(dis, colormap, &xcolor);
      // scalevector[i] = i;
    }  
    break;
  case Grayscale:
    announce("GRAYSCALE\n");
    for (i=0; i<Nscale; ++i) {
      x = 0.75*(1.0-double(i)/double(Nscale))+0.25;
      xcolor.blue = xcolor.green = xcolor.red = short(65500.0*x); 
      test(printf("%lf %d\n", x, xcolor.red));
      xcolor.pixel = scalevector[i];
      xcolor.flags = DoRed | DoGreen | DoBlue;
      XStoreColor(dis, colormap, &xcolor);
    }
    break;
  case Geography:
  default:
    for (i=0; i<Nscale; ++i) scalevector[i] = defpixel;
  }
}

unsigned long FarbPalette::scale(double x)
{
  if (!scalevector || Nscale==0) return defpixel;
  //double dx = (smax-smin)/Nscale;
  int i = int((x-smin+0.5*sdx)/sdx);
  if (i<=0) return scalevector[0];
  else if (i<Nscale) return scalevector[i];
  else return scalevector[Nscale-1];
}

void FarbPalette::drawscale(Drawable drw, GC gc, int x, int y,
			     unsigned int width, unsigned int height) {
  if (width>height) { // horizontal
    unsigned int d = width/Nscale;
    for (int i=0; i<=Nscale; ++i) {
      XSetForeground(dis, gc, scalevector[i]); 
      XFillRectangle(dis, drw, gc, x+i*d, y, d, height);
    }
  }
  else { // vertical
    unsigned int d = height/Nscale;
    for (int i=0; i<=Nscale; ++i) {
      XSetForeground(dis, gc, scalevector[i]); 
      XFillRectangle(dis, drw, gc, x, y+i*d, width, d);
    }
  }
}





#define GCmyMask (GCForeground | GCBackground | GCLineStyle | GCLineWidth)


void Input(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *wD) {
  f->input(wD->event);
  test(printf("input callback window 1:%d 2:%d 3:%d\n",wD->event->xany.window,
	      XtWindow(w), f->win));
}

//------ global callback functions

Fenster::Fenster() {
  controlPanel = NULL;
  controlPanelName = NULL;
  dis = NULL;
  fontstruct = NULL;
}

void Fenster::input(XEvent *event) {
  XWindowAttributes att;
  int x, y, n;
  unsigned int w, h;
  Arg args[6];
  //wD = (XmDrawingAreaCallbackStruct*)p3;
  n = 0;
  XtSetArg(args[n], "x", &x); n++;
  XtSetArg(args[n], "y", &y); n++; 
  XtGetValues(widget, args, n);
 
  switch(event->type) {
  case MappingNotify:
    XRefreshKeyboardMapping((XMappingEvent*)event);
    break;
  case ButtonPress:
    switch(event->xbutton.button) {
    case 1:
      /* Objekt anklicken */
      printf("Mouse button 1 pressed\n");
      /*  XtManageChild(main_window); */
      break;
    case 2:
      /* PopupMenu aktivieren */
      break;
    case 3:
      //MrmFetchWidget(s_MrmHierarchy, f->controlPanelName, f->widget, 
      // &(f->controlPanel), &class_id); 
      if (controlPanel) {
	//gui->Error("unable to fetch control panel widget");
	n = 0;
	XtSetArg(args[n], "width", &w); n++;
	XtSetArg(args[n], "height", &h); n++; 
	XtGetValues(controlPanel, args, n); 
	n = 0;
	XtSetArg(args[n], "x", x + event->xbutton.x - w/2); n++;
	XtSetArg(args[n], "y", y + event->xbutton.y - h/2); n++; 
	XtSetArg(args[n], "userData", (unsigned long)this); n++;
	XtSetValues(controlPanel, args, n);
	XtManageChild(controlPanel);
      }
      break;
    default:
      XGetWindowAttributes(dis, win, &att); 
      x = event->xbutton.x - att.x;
      y = event->xbutton.y - att.y;
      /*  XSetBackground(mydisplay, mygc, 0); */
      printf("Button %i pressed at (%d, %d)\n", event->xbutton.button, x, y);
      /* XDrawImageString(f->dis, f->win, f->gc[0], x, y, hi, strlen(const hi)); */
      break;
    }   
    break;
  case KeyPress:
    printf("keypress\n"); 
    break;
  }
}

void Exposure(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *cbs)
{
  XEvent *event = cbs->event;
  if (event) {
    test(printf("exposure callback window 1:%d 2:%d 3:%d\n",event->xany.window,
		XtWindow(w), f->win));
    f->win = event->xany.window; 
    //    f->win = event->xany.window;
    //    f->win = XtWindow(w);
    if( event->xexpose.count == 0) {
      f->draw();
    }
  }
}

void Resize(Widget w, Fenster *f, XmDrawingAreaCallbackStruct *cbs) {
  XEvent *event = cbs->event;
  if (event) {
    f->win = event->xexpose.window; 
//    test(cout << "Resize\n");
    f->draw();
  }
}

void Scroll(Widget w, Fenster* f, XmScrollBarCallbackStruct *cbs) {
  //  if (f->scrollpermit()) {
  //  test(printf("Scroll\n"));
  f->draw();
  // }
  // else test(printf("don't scroll."));
}

//------ FENSTER
void Fenster::draw() {printf("Basic Class, nothing to draw.\n");}

void Fenster::redraw() {}

void Fenster::panel () {}

void Fenster::panel (Widget *W, char *S) {
  W = &controlPanel;
  S = controlPanelName;
}

/* end of file */












