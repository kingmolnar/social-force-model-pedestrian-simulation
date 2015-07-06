#define NTEST
#include "test.h"

#ifndef MOTIF
#error Symbol MOTIF not defined!
#endif
#include "network.hh"
#include "netfenster.hh"

#ifndef NULL
#define NULL 0
#endif
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>

NetFarbScale::NetFarbScale(Display *d, Window w, int numbersc) {
  XColor color;
  dis = d;
  screen = XDefaultScreen(dis);
  colormap = XDefaultColormap(dis, screen);
  //  visual = DefaultVisual(dis, screen);
  //colormap = XCreateColormap(dis, w, DefaultVisual(dis, screen), AllocAll);
  XSetWindowColormap(dis, w, colormap);
  
  /* color.red = color.green = 0; color.blue = 65535;
     color .flags = DoRed | DoGreen | DoBlue;
     //  XAllocColor(dis, colormap, &color);
     blackpix = color.pixel = Nscale+1;
     XStoreColor(dis, colormap, &color);
  
     color.red = color.green = color.blue = 32768;
     color .flags = DoRed | DoGreen | DoBlue;
     // XAllocColor(dis, colormap, &color);
     graypix = color.pixel = Nscale+2; 
     XStoreColor(dis, colormap, &color);
     
     color.red = color.green = color.blue = 65535;
     color .flags = DoRed | DoGreen | DoBlue;
     //  XAllocColor(dis, colormap, &color);
     whitepix = color.pixel = Nscale+3;
     XStoreColor(dis, colormap, &color);
     */
  blackpix = BlackPixel(dis, screen);
  whitepix = WhitePixel(dis, screen);
  defpixel = BlackPixel(dis, screen);
  if (numbersc%2==1) Nscale = numbersc;
  else Nscale = numbersc+1;
  scalevector = new unsigned long[Nscale+1];
  // allocate colors
  //  XColor *colors = new XColor[Nscale+1];

  unsigned long plane_masks[1] ;
  while (1) {
    if ( XAllocColorCells (dis, colormap, False,
			   plane_masks, 0, scalevector, Nscale+1) )
      break ;
    Nscale-- ;
    if (Nscale == 0) {
      //      exit (0) ;
    }
  }
  //for (int i=0; i<=Nscale; ++i) {
  //scalevector[i] = colors[i].pixel;
  // }
  //delete colors
}

NetFarbScale::~NetFarbScale() {
  delete scalevector;
  XFreeColormap(dis, colormap);
}

void NetFarbScale::limits(double min, double max, ScaleStyle st) {
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
  switch (st) {
  case UNDEF:
    // nichts aendern!
    break;
  case Temperature: // blau-weiss-rot
  case Rainbow: // blau-gruen-gelb-rot
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
  case Geography:
  default:
    for (i=0; i<Nscale; ++i) scalevector[i] = defpixel;
  }
}

unsigned long NetFarbScale::scale(double x)
{
  if (scalevector && Nscale && x>=smin && x<=smax) {
    double dx = (smax-smin)/Nscale;
    int i = int((x-smin+0.5*dx)/dx);
    if (scalevector && i>=0 && i<=Nscale) return scalevector[i];
    else return defpixel;
  }
  else return defpixel;
}

void NetFarbScale::drawscale(Drawable drw, GC gc, int x, int y,
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

NetFenster::NetFenster(Display *display, Network *net) {
#define MAX_ARGS 8
  Widget mainwindow, frame, scroll; /*  verti, hori,*/ 
  Arg args[MAX_ARGS];
  Cardinal n;
  char text[80];
  Window root;
  int x, y, i, j, k;
  unsigned int bord, depth;
  int h = 600, w = 800;
  XGCValues xgcv;
  XColor xcolor, xcolor_exact;
  XSizeHints hint;
  XSetWindowAttributes myattr;
  unsigned long int mymask;
  FILE *fp;
  Status status; 
  static double perc[] = { 0.5, 0.67, 0.83, 1.0 }; 
  ParameterDef d[] = { {"colored", "1"},
		       {"pixpm" , "20"},
		       {"linewidth", "5"},
		     //  {"title" ,"\'Schopping_Mall"}, {"white", "\'white"},
		     //  {"black", "\'black"}, {"grey", "\'grey"},
		     //  {"col2",  "\'red"}, {"col3",   "\'green"},
		     //  {"col4",  "\'blue"}, {"col5",  "\'cyan"},
		     //  {"col6", "\'magenta"},
		       {"x", "0"}, {"y", "0"},
		       {"width", "800"}, {"height", "600"}
		     };
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  parameter->b = new bool[4];
  parameter->b[0] = 0;
  parameter->b[3] = 1;
  network = net;
  n = 0;
  nr = FensterNummer++;
  // Init
  dis = display;
  vis = 0;
  farbe = 0;
  gc = 0;
  parameter->get("height", &h);
  parameter->get("width", &w);
  parameter->get("x", &px);
  parameter->get("y", &py);
  parameter->get("pixpm", &pixpm);
  parameter->get("linewidth", &linewidth);

  // Configuration
  range.x1 = network->Xmin.x-1.0;
  range.y1 = network->Xmin.y-1.0;
  range.x2 = network->Xmax.x+1.0;
  range.y2 = network->Xmax.y+1.0;
  width = (int)(range.x2 - range.x1)*pixpm;
  if (width<w) width = w;
  height = (int)(range.y2 - range.y1)*pixpm;
  if (height<h) height = h;
    
  old.width = old.height = old.px = old.py = 0;
  font = XLoadFont(dis, "terminal14");

  // Widgets  
  n = 0;
  XtSetArg(args[n], "allowShellResize", True);  n++;
  XtSetArg(args[n], "height", h); n++;
  XtSetArg(args[n], "width", w); n++;
  sprintf(text, "Network");  
  widget = XtAppCreateShell(text, NULL, 
			    applicationShellWidgetClass,
			    dis, args, n);

  sprintf(text, "main%04d", nr);
  n = 0;
  XtSetArg(args[n], "height", h); n++;
  XtSetArg(args[n], "width", w); n++;
  mainwindow = XmCreateMainWindow(widget, text, args, n);
  XtManageChild(mainwindow);
     
  sprintf(text, "message%04d", nr);
  n = 0;
  XtSetArg(args[n], "value", "(c) Peter Molnar 1992-94"); n++;
  XtSetArg(args[n], "editable", False); n++;
  XtSetArg(args[n], "cursorPositionVisible", False); n++;
  message = XmCreateText(mainwindow, text, args, n);
  XtManageChild(message); 
  XtAddCallback(message, "inputCallback", Input, this);
  // bsp: XmTextSetString(Widget, char*);
  //      char *txt = XmTextGetString(Widget); XtFree(txt) 

  sprintf(text, "frame%04d", nr);
  n = 0;
  frame = XmCreateFrame(mainwindow, text, args, n);
  XtManageChild(frame); 

  sprintf(text, "verti%04d", nr);
  n = 0;
  XtSetArg(args[n], "userData", 1); n++;
  verti = XmCreateScrollBar(mainwindow /* scroll */ , text, args, n);
  XtManageChild(verti);
  XtAddCallback(verti, XmNvalueChangedCallback, Scroll, this);
  
  sprintf(text, "hori%04d", nr);
  n = 0;
  XtSetArg(args[n], "orientation", XmHORIZONTAL); n++;
  XtSetArg(args[n], "userData", 0); n++;
  hori = XmCreateScrollBar(mainwindow, text, args, n);
  XtManageChild(hori);
  XtAddCallback(hori, XmNvalueChangedCallback, Scroll, this);
  
  sprintf(text, "draw%04d", nr);
  n = 0;
  XtSetArg(args[n], "width", w); n++; // ehem.  width
  XtSetArg(args[n], "height", h); n++; // ehem.  height 
  XtSetArg(args[n], "resize", False); n++;
  area = XmCreateDrawingArea(frame, text, args, n);
  XtManageChild(area);
  XtAddCallback(area, "exposeCallback", Exposure, this);
  XtAddCallback(area, "inputCallback", Input, this);
  
 // XmScrolledWindowSetAreas(scroll, hori, verti, area);
  XmMainWindowSetAreas(mainwindow, NULL, message, hori, verti, frame);

  //controlPanelName = "WindowControlNet";
  // es geht los!
  XtRealizeWidget(widget);
}

NetFenster::~NetFenster()
{
  int i, j, k;
  if (widget) {
    XtUnrealizeWidget(widget);
    XtDestroyWidget(widget);
    widget = NULL;
  }
  if (vis) XFreePixmap(dis, vis); vis = NULL;
}

#define GCmyMask (GCForeground | GCBackground | GCLineStyle | GCLineWidth)

void NetFenster::draw()
{
  Arg args[6];
  int n;
  unsigned long mypixel;
  bool _size, _scroll;
  if (!win) {
    printf("kein Window!\n"); return;
  }
  if (!farbe) {
    farbe = new NetFarbScale(dis, win, 31);
  }
  else {
    XSetWindowColormap(dis, win, farbe->colormap);
  }
  if (!gc) {
    Ngc = 1;
    gc = new GC[Ngc];
    XGCValues xgcv;
    XSetWindowAttributes attrs;
    attrs.bit_gravity = ForgetGravity;
    XChangeWindowAttributes(dis, XtWindow(area), CWBitGravity, &attrs);
    xgcv.foreground = farbe->black();
    xgcv.background = farbe->white();
    xgcv.line_width = linewidth;
    xgcv.line_style = LineSolid;
    gc[0] = XCreateGC(dis, win, GCmyMask, &xgcv);
    //xgcv.line_style = LineOnOffDash;
    //gc[1] = XCreateGC(dis, win, GCmyMask, &xgcv); Ngc = 2;
  }
  XGetWindowAttributes(dis, win, &a);
  scrpermit = FALSE;
  if (hori) {
    if (a.width>=width) {
      XtUnmanageChild(hori);
      px = 0;
    }
    else {
      n = 0;
      XtSetArg(args[n], "value", &px); n++;
      XtGetValues(hori, args, n);
      n = 0;
      XtSetArg(args[n], "increment", 2*pixpm); n++;
      XtSetArg(args[n], "sliderSize", a.width<width ? a.width : width); n++;
      XtSetArg(args[n], "pageIncrement", a.width); n++;
      XtSetArg(args[n], "minimum", 0); n++;
      XtSetArg(args[n], "maximum", width); n++;
      XtSetValues(hori, args, n); // ruft scroll auf, setzt px
      XtManageChild(hori);
    } 
  }
  if (verti) {
    if (a.height>=height) {
      XtUnmanageChild(verti);
      py = 0;
    }
    else {
      n = 0;
      XtSetArg(args[n], "value", &py); n++;
      XtGetValues(verti, args, n);
      n = 0;
      XtSetArg(args[n], "increment", 2*pixpm); n++;
      XtSetArg(args[n], "sliderSize", a.height<height ? a.height:height); n++;
      XtSetArg(args[n], "pageIncrement", a.height); n++;
      XtSetArg(args[n], "minimum", 0); n++;
      XtSetArg(args[n], "maximum", height); n++;
      XtSetValues(verti, args, n); // ruft scroll auf, setzt py
      XtManageChild(verti);
    }
  }
  scrpermit = TRUE;
  
  // create grafik etc.
  _size = (a.width!=old.width || a.height!= old.height);
  _scroll = (px!=old.px || py!=old.py);
  if (_size || _scroll) {
    // neuen Hintergrund
    old.width = a.width; old.height = a.height; old.px = px; old.py = py;
    if (_size) {
      if (vis) XFreePixmap(dis, vis);
      vis = XCreatePixmap(dis, win, a.width, a.height, a.depth);
    } 
    XSetForeground(dis, gc[0], farbe->white()); 
    XFillRectangle(dis, vis, gc[0], 0, 0, a.width, a.height);
    network->draw(this, vis);
    if (parameter->b[3]) {
      // freiwillig scale
      farbe->drawscale(vis, gc[0], a.width-20, 0, 20, a.height);
    }
  }
  if (win && vis) {
    XCopyArea(dis, vis, win, gc[0], 0, 0, a.width, a.height, 0,0);
  }

  /* {
     char text[20];
     sprintf(text, "time %5.1lf", mall->T);
     XmTextSetString(message, text);
     } */
}

void Network::draw(NetFenster *f, Drawable drw) {
  double lw;
  char text[10];
  Pointer<EdgeClass> ed;
  Pointer<VertexClass> v;
  int freqmax = 0;
  for (ed=edges; ed; ++ed) {
    if (ed->frequency>freqmax) freqmax = ed->frequency;
  }    
  //if (f->parameter->b[0]) { //color
  f->farbe->limits(0, freqmax, Rainbow);
  //}
  int npoints, i;
  for (ed=edges; ed; ++ed) {
    npoints = ed->vertices.N();
    i = 0;
    XPoint *points = new XPoint[npoints];
    for (v=ed->vertices; v; ++v) {
      points[i].x = f->xx(v->P.x);
      points[i].y = f->yy(v->P.y);
      test(printf("point%d (%d,%d)\n", i,points[i].x, points[i].y));
      ++i;
    }
    if (npoints!=i) printf("fehler\n");
    // XFillPolygon(f->dis, drw, f->gc[0], points, npoints,
    // Complex, CoordModeOrigin);
    if (f->parameter->b[0]) { //color
      XSetForeground(f->dis, f->gc[0], f->farbe->scale(ed->frequency));
    }
    else { //black&white
      lw = 3.0 * double(f->linewidth) * ed->frequency/freqmax + 0.5;
      XSetLineAttributes(f->dis, f->gc[0], int(lw),
			 LineSolid, CapRound, JoinRound);
      XSetForeground(f->dis, f->gc[0], f->farbe->black());
    }
    XDrawLines(f->dis, drw, f->gc[0], points, npoints, CoordModeOrigin);
    delete points;
  } 
  XSetForeground(f->dis, f->gc[0], f->farbe->black());
  int myind;
  /* for (v=vertices; v; ++v) {
     myind = v->index();
     sprintf(text, "%d", myind);
     XDrawImageString(f->dis, drw, f->gc[0], f->xx(v->P.x), f->yy(v->P.y), 
     text, strlen(text));
     } */
}
/* end of file */












