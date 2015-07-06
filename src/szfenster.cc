#define NTEST
#include "test.h"
#include "szfenster.hh"
#define XWINDOWS
#include "gui.hh"

#include "pedestrian.hh"
#include "floor.hh"
#include "street.hh"
#include "ground.hh"
#include "movie.hh"

#ifndef NULL
#define NULL (*void)0
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

externalref WidgetClass applicationShellWidgetClass;

#define XmVERTICAL	       1
#define XmHORIZONTAL           2


extern GUI *userinterface;

#define GCmyMask (GCForeground | GCBackground | GCLineStyle | GCLineWidth)

SimulObject* newSzeneFenster() {return new SzeneFenster();}
SimulObject* newVideoFenster() {return new VideoFenster();}
SimulObject* newReplayFenster() {return new ReplayFenster();}

//SzeneFenster::SzeneFenster(Display *display, Floor *m) {
SzeneFenster::SzeneFenster() {
  dis = userinterface->display;
  nr = FensterNummer++;
  floor = NULL;
  ground = NULL;
  symbol.arrow = NULL;
  symbol.circle = NULL;
  movie = NULL;
  widget = NULL;
  pim = vis = 0;
  farbe = 0;
  farbenliste = NULL;
  gc[0] = gc[1] = 0;
  controlPanelName = "WindowControlPed";
 
  ParameterDef d[] = { {"colored", "1"},
		     {"pixpm" , "15"},
		     {"linewidth", "3"},
		     {"pedwidth", "0.8"},
		     {"arrowlength", "2.3"},
		     {"arrowwidth", "0.7"},
		     {"arrowratio", "0.3"},
		     {"flag", "1063"},
		     {"x", "0"}, {"y", "0"},
		     {"width", "800"}, {"height", "600"}
		       //  {"title" ,"\'Schopping_Floor"},
		       //  {"white", "\'white"},
		       //  {"black", "\'black"},
		       //  {"grey", "\'grey"},
		       //  {"col2",  "\'red"},
		       //  {"col3",   "\'green"},
		       //  {"col4",  "\'blue"},
		       //  {"col5",  "\'cyan"},
		       //  {"col6", "\'magenta"},
		       
		     };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  intszflag = 0;
  szflag.color = 1; // 0 
  szflag.obstac = 1; // 1
  szflag.attrac = 1; // 2
  szflag.walker = 2; // 3 4 5
  szflag.trace  = 0; // 6
  szflag.ground = 0; // 10
//  parameter->create = intszflag;
  //objecttype = _SCENERY_WINDOW;
  test(printf("szenefenster pointer=%d\n", this));
}

SzeneFenster::~SzeneFenster()
{
  int i, j, k;
  if (widget) {
    XtUnrealizeWidget(widget);
    XtDestroyWidget(widget);
    widget = NULL;
  }
  if (vis) XFreePixmap(dis, vis); vis = NULL;
  if (pim) XFreePixmap(dis, pim); pim = NULL;
  /* if (symb) {
    for (i=0; i<NCOLOR; i++) 
    for (j=0; j<4; j++) 
    for (k=0; k<4; k++) 
    if (symb_pim(i,j,k)) {
    XFreePixmap(dis, symb_pim(i,j,k)); 
    symb_pim(i,j,k) = NULL;
    }
    free(symb); f->symb = NULL;
    } */
}

void SzeneFenster::init() {
  announce("SzeneFenster::init()\n");
  Fenster::init();
  test(printf("SzeneFenster::floor=%d ground=%d\n", floor, ground));
  if (!floor && !ground) {
    if (root && root->floors.N()) {
      Pointer<Floor> fl = root->floors;
      floor = fl();
    }
  }
#define MAX_ARGS 8
  Widget mainwindow, frame, scroll; /*  verti, hori,*/ 
  Arg args[MAX_ARGS];
  //  XtAppContext    app_context;	// application context 
  Cardinal n;
  char text[80];
  Window rootwin;
  int x, y, i, j, k;
  unsigned int bord, depth;
  XGCValues xgcv;
  XColor xcolor, xcolor_exact;
  XSizeHints hint;
  XSetWindowAttributes myattr;
  unsigned long int mymask;
  FILE *fp;
  Status status; 
  static double perc[] = { 0.5, 0.67, 0.83, 1.0 }; 
  
  //int fl = parameter->flags;
  int temp;
  get(parameter, "symbols",    &temp, 1); szflag.walker       = temp;
  get(parameter, "obstac",     &temp, 1); szflag.obstac       = temp; 
  get(parameter, "attrac",     &temp, 1); szflag.attrac       = temp; 
  get(parameter, "trace",      &temp, 0); szflag.trace        = temp; 
  get(parameter, "gro",     &temp, 0); szflag.ground       = temp; 
  get(parameter, "scrollbars", &temp, 1); szflag.noscrollbars = temp ? 0 : 1; 
  //  get(parameter, "flag", &intszflag, intszflag);
  int h, w;
  get(parameter, "height", &h, 600);
  get(parameter, "width", &w, 800);
  parameter->get("x", &px);
  parameter->get("y", &py);
  parameter->get("pixpm", &pixpm);
  parameter->get("linewidth", &linewidth);
  parameter->get("pedwidth", &pedwidth);
  char *fontname;
  get(parameter, "font", &fontname, "courier12");

//   if (parameter->changed("font")) {
//     fontstruct = XLoadQueryFont(dis, fontname);
//     if (fontstruct) font = fontstruct->fid;
//     else {
//       fontstruct = XLoadQueryFont(dis, "fixed");
//       if (fontstruct) font = fontstruct->fid;
//       else {
// 	// default font 
//       }
//     }
//   }
  
  get(parameter, "colors", &farbenliste, "");

  // init Symbols 
  {
    if (!symbol.arrow) symbol.arrow = new GraphicArrow(this);
    double vmax = floor? floor->vmax: 1.5;
    double l, w, r, m;
    get(parameter, "vmax", &m, vmax);
    get(parameter, "arrowlength", &l, 1.0);
    get(parameter, "arrowwidth", &w, 0.7);
    get(parameter, "arrowratio", &r, 0.3);
    symbol.arrow->scale(l, w, r, m);
  }
  // Configuration
  if (floor) {
    range.x1 = floor->Xmin.x;
    range.y1 = floor->Xmin.y;
    range.x2 = floor->Xmax.x;
    range.y2 = floor->Xmax.y;
  }
  /*   else if (ground) {
       range.x1 = ground->Xmin.x;
    range.y1 = ground->Xmin.y;
    range.x2 = ground->Xmax.x;
    range.y2 = ground->Xmax.y;
    } */

  width = 10 + (int)(range.x2 - range.x1)*pixpm;
  if (width<w) w = width;
  height = 30 + (int)(range.y2 - range.y1)*pixpm;
  if (height<h) h = height;
  
    
  old.width = old.height = old.px = old.py = 0;
  
  
  // Widgets
  if (!widget) {
    n = 0;
    XtSetArg(args[n], "allowShellResize", True);  n++;
    XtSetArg(args[n], "height", h); n++;
    XtSetArg(args[n], "width", w); n++;
    sprintf(text, "Dynamic Force Model for the Behaviour of Pedestrians");  
    widget = XtAppCreateShell(text, NULL, 
			      applicationShellWidgetClass,
			      dis, args, n);
    
    sprintf(text, "main%04d", nr);
    n = 0;
    XtSetArg(args[n], "height", h); n++;
    XtSetArg(args[n], "width", w); n++;
    //XtSetArg(args[n], XmNmessageWindow, message); n++;
    mainwindow = XmCreateMainWindow(widget, text, args, n);
    XtManageChild(mainwindow);
    
//     sprintf(text, "message%04d", nr);
//     n = 0;
//     XtSetArg(args[n], "value", "(c) Peter Molnar 1992-94"); n++;
//     XtSetArg(args[n], "editable", False); n++;
//     XtSetArg(args[n], "cursorPositionVisible", False); n++;
//     message = XmCreateText(mainwindow, text, args, n);
//     XtManageChild(message); 
//     XtAddCallback(message, "inputCallback", Input, this);
//     // bsp: XmTextSetString(Widget, char*);
//     //      char *txt = XmTextGetString(Widget); XtFree(txt) 
    
    sprintf(text, "frame%04d", nr);
    n = 0;
    frame = XmCreateFrame(mainwindow, text, args, n);
    XtManageChild(frame); 
    
    /*   sprintf(text, "scroll%04d", nr);
	 n = 0;
	 XtSetArg(args[n], XmNheight, h); n++;
	 XtSetArg(args[n], XmNwidth, w); n++;
	 XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);
	 XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED);
	 scroll = XmCreateScrolledWindow(frame, text, args, n);
	 XtManageChild(scroll);
	 */
    sprintf(text, "verti%04d", nr);
    n = 0;
    XtSetArg(args[n], "userData", 1); n++;
    verti = XmCreateScrollBar(mainwindow, text, args, n);
    XtManageChild(verti);
    XtAddCallback(verti, "valueChangedCallback", Scroll, this);
    
    sprintf(text, "hori%04d", nr);
    n = 0;
    XtSetArg(args[n], "orientation", XmHORIZONTAL); n++;
    XtSetArg(args[n], "NuserData", 0); n++;
    hori = XmCreateScrollBar(mainwindow /* scroll */, text, args, n);
    XtManageChild(hori);
    XtAddCallback(hori, "valueChangedCallback", Scroll, this);
    
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
    //   XmMainWindowSetAreas(mainwindow, NULL, message, hori, verti, frame);
    XmMainWindowSetAreas(mainwindow, NULL, NULL, hori, verti, frame);
 
    XtRealizeWidget(widget);
    win = XtWindow(area);
  }
}

int SzeneFenster::ref(SimulObject *obj, SimulObjectType type) {
  test(printf("SzeneFenster::ref type=%d\n", type));
  int good = 0;
  if (!obj) return good;
  switch (type) { 
  case _GROUND:
    announce("Ground Ref\n");
    ground = (Ground*)obj;
    good = 1;
    break;
  case _FLOOR:
    announce("Floor Ref\n");
    floor = (Floor*)obj;
    good = 1;
    break;
  case _MOVIE:
    movie = (Movie*)obj;
    good = 1;
    break;
  default:
    good = 0;
    break;
  }
  return good ? 1 : Fenster::ref(obj, type);
}

void SzeneFenster::draw()
{
  Arg args[6];
  int n;
  unsigned long mypixel;
  char hi[] = "Hallo Peter!";
  bool _size, _scroll;
  if (!farbe) {
    farbe = new FarbPalette(dis);
    if (farbenliste) farbe->names(farbenliste);
  }
  if (!win) {
    printf("kein Window!\n"); return;
  }
  if (!gc[0]) {
    XGCValues xgcv;
    XSetWindowAttributes attrs;
    attrs.bit_gravity = ForgetGravity;
    XChangeWindowAttributes(dis, XtWindow(area), CWBitGravity, &attrs);
    xgcv.foreground = farbe->foreground();
    xgcv.background = farbe->background();
    xgcv.line_width = linewidth;
    xgcv.line_style = LineSolid;
    gc[0] = XCreateGC(dis, win, GCmyMask, &xgcv);
    xgcv.line_style = LineOnOffDash;
    gc[1] = XCreateGC(dis, win, GCmyMask, &xgcv);
    Ngc = 2;
  }
  XGetWindowAttributes(dis, win, &a);
  // test(cout << "attributes\n"); 
  /* Fensterinhalt rekonstruieren, evtl. abh. von d. Einstellung */
  //!!!!redraw();
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
  if (_size || _scroll || szflag.ground) {
    // neuen Hintergrund
    old.width = a.width; old.height = a.height; old.px = px; old.py = py;
    if (_size) {
      if (pim) XFreePixmap(dis, pim);
      if (vis) XFreePixmap(dis, vis);
      pim = XCreatePixmap(dis, win, a.width, a.height, a.depth);
      vis = XCreatePixmap(dis, win, a.width, a.height, a.depth);
    } 
    
#undef STREIFIGERHINTERGRUND
#ifdef STREIFIGERHINTERGRUND
    for (int ii=0; ii<8; ii++) {
      XSetForeground(dis, gc[0], farbe->pix(ii)); 
      XFillRectangle(dis, pim, gc[0], ii*a.width/8, 0,
		     a.width/8, a.height);
    }
#else // normal
    mypixel = farbe->background();
    XSetForeground(dis, gc[0], mypixel); 
    XFillRectangle(dis, pim, gc[0], 0, 0, a.width, a.height);
#endif

    if (szflag.ground) {
      if (ground) { //Ground 
	ground->draw(this, pim);
      }
      else if (movie && movie->movieIn()) {
	Pointer<Ground> gro = movie->grounds;
	if (gro) gro->draw(this, pim);
      }
      else if (floor) {
	Pointer<Ground> gro = floor->grounds;
	if (gro) gro->draw(this, pim);
      }
    }
    
#ifdef _STREET_HH_
    if (szflag.obstac && floor) { //Street
      for (Pointer<Street> street=floor->streets; street; ++street) {
	street->draw(this, pim); // alle Strassen Malen
      }
      for (Pointer<Gate> gat=floor->gates; gat; ++gat) {
	gat->draw(this, pim); // alle Tore
      } 
    }
#endif
  } // ende hintergrund
  
  XCopyArea(dis, pim, vis, gc[0], 0, 0, a.width, a.height, 0, 0);    
  //peds
  if (szflag.walker) {
    announce("pedestrians\n");
    if (movie && movie->movieIn() && movie->walkers.N()) {
      for (Pointer<Walker> wlk=movie->walkers; wlk; ++wlk) {
	wlk->draw(this, vis);
      }
    }
    else if (floor && floor->pedestrians.N()) {
      for (Pointer<Pedestrian> ped=floor->pedestrians; ped; ++ped) {
	ped->draw(this, vis); 
      }
    }
    else if (movie && movie->movieIn() && movie->floors.N()) {
      Pointer<Floor> mflo=movie->floors;
      if (mflo->pedestrians.N()) { 
	for (Pointer<Pedestrian> ped=mflo->pedestrians; ped; ++ped) {
	  ped->draw(this, vis);
	}
      }
    }
    else if (floor) {
      for (Pointer<Street> street=floor->streets; street; ++street) {
	for (Pointer<Pedestrian> ped=street->pedestrians; ped; ++ped) {
	  ped->draw(this, vis); 
	}
      }
    } 
  }
  char text[20];
  if (movie && movie->movieIn()) {
    if (movie->ismarked()) sprintf(text, "time %5.1lf (*)", movie->T());
    else sprintf(text, "time %5.1lf", movie->T());
  }
  else {
    sprintf(text, "time %5.1lf", floor ? floor->T : 0.0);
  }
  if (text) {
    int textlen = strlen(text);
    int d, asc = 10, desc = 5;
//     XCharStruct overall;
//     if (fontstruct) {
//       XTextExtents(fontstruct, text, textlen, &d, &asc, &desc, &overall);
//       XSetFont(dis, gc[0], font);
//     }
    XSetForeground(dis, gc[0], farbe->foreground());
    XDrawString(dis, vis, gc[0], 8, 8+asc, text, textlen);
  }
  if (win && vis) {
    XCopyArea(dis, vis, win, gc[0], 0, 0, a.width, a.height, 0,0);
  }
  // XmTextSetString(message, text);
}

void SzeneFenster::redraw() {
  if (win && vis) {
    XCopyArea(dis, vis, win, gc[0], 0, 0, a.width, a.height, 0,0);
  }
  else draw(); 
} 


VideoFenster::VideoFenster() {
}

VideoFenster::~VideoFenster() {
}

void VideoFenster::init() {
  announce("VideoFenster::init()\n");
  Fenster::init();
#define MAX_ARGS 8
  Widget mainwindow, frame; /*  verti, hori,*/ 
  Arg args[MAX_ARGS];
  //  XtAppContext    app_context;	// application context 
  Cardinal n;
  char text[80];
  Window rootwin;
  int x, y, i, j, k;
  unsigned int bord, depth;
  XGCValues xgcv;
  XColor xcolor, xcolor_exact;
  XSizeHints hint;
  XSetWindowAttributes myattr;
  unsigned long int mymask;
  FILE *fp;
  Status status; 
  static double perc[] = { 0.5, 0.67, 0.83, 1.0 }; 
  
  int temp;
  get(parameter, "symbols",    &temp, 1); szflag.walker       = temp;
  get(parameter, "obstac",     &temp, 1); szflag.obstac       = temp; 
  get(parameter, "attrac",     &temp, 1); szflag.attrac       = temp; 
  get(parameter, "trace",      &temp, 0); szflag.trace        = temp; 
  get(parameter, "gro",     &temp, 0); szflag.ground       = temp; 
  get(parameter, "scrollbars", &temp, 1); szflag.noscrollbars = temp ? 0 : 1; 
  // get(parameter,"flag", &intszflag, intszflag);
  parameter->get("x", &px);
  parameter->get("y", &py);
  parameter->get("pixpm", &pixpm);
  parameter->get("linewidth", &linewidth);
  parameter->get("pedwidth", &pedwidth);
  char *fontname;
  get(parameter, "font", &fontname, "courier12");

//   if (parameter->changed("font")) {
//     fontstruct = XLoadQueryFont(dis, fontname);
//     if (fontstruct) font = fontstruct->fid;
//     else {
//       fontstruct = XLoadQueryFont(dis, "fixed");
//       if (fontstruct) font = fontstruct->fid;
//       else {
// 	// default font 
//       }
//     }
//   }
  
  get(parameter, "colors", &farbenliste, "");

  // init Symbols 
  {
    if (!symbol.arrow) symbol.arrow = new GraphicArrow(this);
    double vmax = floor? floor->vmax: 1.5;
    double l, w, r, m;
    get(parameter, "vmax", &m, vmax);
    get(parameter, "arrowlength", &l, 1.0);
    get(parameter, "arrowwidth", &w, 0.7);
    get(parameter, "arrowratio", &r, 0.3);
    symbol.arrow->scale(l, w, r, m);
  }
  // Configuration

  Vector R1,R2;  
  if (floor) { R1 = floor->Xmin;   R2 = floor->Xmax; }
  // else if (ground) { R1 = ground->Xmin;   R2 = ground->Xmax; }
  
  get(parameter, "xmin", &R1, R1);
  get(parameter, "xmax", &R2, R2);
  range.x1 = R1.x;
  range.y1 = R1.y;
  range.x2 = R2.x;
  range.y2 = R2.y;
  int w = width = int((range.x2 - range.x1)*pixpm);
  int h = height = int((range.y2 - range.y1)*pixpm);

  /* get(parameter, "height", &h, int(height));
     get(parameter, "width", &w, int(width));
  */
  
  /* Vector DX;
     get(parameter, "size", &DX, Vector(10.0,10.0));
     int w = width = int(DX.x * pixpm);
     int h = height = int(DX.y * pixpm);
  */

  old.width = old.height = old.px = old.py = 0;
  
  
  // Widgets
  if (!widget) {
    n = 0;
    //XtSetArg(args[n], "allowShellResize", True);  n++;
    XtSetArg(args[n], "height", h); n++;
    XtSetArg(args[n], "width", w); n++;
    sprintf(text, "Dynamic Force Model for the Behaviour of Pedestrians");  
    widget = XtAppCreateShell(text, NULL, 
			      applicationShellWidgetClass,
			      dis, args, n);
    
    sprintf(text, "main%04d", nr);
    n = 0;
    XtSetArg(args[n], "height", h); n++;
    XtSetArg(args[n], "width", w); n++;
    //XtSetArg(args[n], XmNmessageWindow, message); n++;
    mainwindow = XmCreateMainWindow(widget, text, args, n);
    XtManageChild(mainwindow);
    
    
    //sprintf(text, "frame%04d", nr);
    //n = 0;
    //frame = XmCreateFrame(mainwindow, text, args, n);
    //XtManageChild(frame); 
    
    sprintf(text, "draw%04d", nr);
    n = 0;
    XtSetArg(args[n], "width", w); n++; // ehem.  width
    XtSetArg(args[n], "height", h); n++; // ehem.  height 
    XtSetArg(args[n], "resize", False); n++;
    
    area = XmCreateDrawingArea(mainwindow/*frame*/, text, args, n);
    XtManageChild(area);
    XtAddCallback(area, "exposeCallback", Exposure, this);
    XtAddCallback(area, "inputCallback", Input, this);
    
    // XmScrolledWindowSetAreas(scroll, hori, verti, area);
    XmMainWindowSetAreas(mainwindow, NULL, NULL, NULL, NULL, area/*frame*/);
 
    XtRealizeWidget(widget);
    win = XtWindow(area);
  }
}


void VideoFenster::draw() {
  Arg args[6];
  int n;
  unsigned long mypixel;
  char hi[] = "Hallo Peter!";
  bool _size, _scroll;
  if (!farbe) {
    farbe = new FarbPalette(dis);
    if (farbenliste) farbe->names(farbenliste);
  }
  if (!win) {
    printf("kein Window!\n"); return;
  }
  if (!gc[0]) {
    XGCValues xgcv;
    XSetWindowAttributes attrs;
    attrs.bit_gravity = ForgetGravity;
    XChangeWindowAttributes(dis, XtWindow(area), CWBitGravity, &attrs);
    xgcv.foreground = farbe->foreground();
    xgcv.background = farbe->background();
    xgcv.line_width = linewidth;
    xgcv.line_style = LineSolid;
    gc[0] = XCreateGC(dis, win, GCmyMask, &xgcv);
    xgcv.line_style = LineOnOffDash;
    gc[1] = XCreateGC(dis, win, GCmyMask, &xgcv);
    Ngc = 2;
  }
  XGetWindowAttributes(dis, win, &a);
  // test(cout << "attributes\n"); 
  /* Fensterinhalt rekonstruieren, evtl. abh. von d. Einstellung */
  //!!!!redraw();
  
  // create grafik etc.
  _size = (a.width!=old.width || a.height!= old.height);
  _scroll = (px!=old.px || py!=old.py);
  if (_size || _scroll || szflag.ground) {
    // neuen Hintergrund
    old.width = a.width; old.height = a.height; old.px = px; old.py = py;
    if (_size) {
      if (pim) XFreePixmap(dis, pim);
      if (vis) XFreePixmap(dis, vis);
      pim = XCreatePixmap(dis, win, a.width, a.height, a.depth);
      vis = XCreatePixmap(dis, win, a.width, a.height, a.depth);
    } 
    
    mypixel = farbe->background();
    XSetForeground(dis, gc[0], mypixel); 
    XFillRectangle(dis, pim, gc[0], 0, 0, a.width, a.height);
    
    if (szflag.ground) {
      if (ground) { //Ground 
	ground->draw(this, pim);
      }
      else if (movie && movie->movieIn()) {
	Pointer<Ground> gro = movie->grounds;
	if (gro) gro->draw(this, pim);
      }
      else if (floor) {
	Pointer<Ground> gro = floor->grounds;
	if (gro) gro->draw(this, pim);
      }
    }
    
#ifdef _STREET_HH_
    if (szflag.obstac && floor) { //Street
      for (Pointer<Street> street=floor->streets; street; ++street) {
	street->draw(this, pim); // alle Strassen Malen
      }
      for (Pointer<Gate> gat=floor->gates; gat; ++gat) {
	gat->draw(this, pim); // alle Tore
      } 
    }
#endif
  } // ende hintergrund
  
  XCopyArea(dis, pim, vis, gc[0], 0, 0, a.width, a.height, 0, 0);    
  //peds
  if (szflag.walker) {
    announce("pedestrians\n");
    if (movie && movie->movieIn() && movie->subpops.N()) {
      for (Pointer<WalkerPop> wop=movie->subpops; wop; ++wop) {
	wop->draw(this, vis);
      }
    }
    else if (floor && floor->pedestrians.N()) {
      for (Pointer<Pedestrian> ped=floor->pedestrians; ped; ++ped) {
	ped->draw(this, vis); 
      }
    }
    else if (movie && movie->movieIn() && movie->floors.N()) {
      Pointer<Floor> mflo=movie->floors;
      if (mflo->pedestrians.N()) { 
	for (Pointer<Pedestrian> ped=mflo->pedestrians; ped; ++ped) {
	  ped->draw(this, vis);
	}
      }
    }
    else if (floor) {
      for (Pointer<Street> street=floor->streets; street; ++street) {
	for (Pointer<Pedestrian> ped=street->pedestrians; ped; ++ped) {
	  ped->draw(this, vis); 
	}
      }
    } 
  }
  // Text
  char text[20];
  if (movie && movie->movieIn()) {
    if (movie->ismarked()) sprintf(text, "%5.1lf sec (*)", movie->T());
    else sprintf(text, "%5.1lf sec", movie->T());
  }
  else {
    sprintf(text, "%5.1lf sec", floor->T);
  }
  //if (messsage) XmTextSetString(message, text);
  if (text) {
    int textlen = strlen(text);
    int d, asc = 10, desc = 5;
//     XCharStruct overall;
//     if (fontstruct) {
//       XTextExtents(fontstruct, text, textlen, &d, &asc, &desc, &overall);
//       XSetFont(dis, gc[0], font);
//     }
    XSetForeground(dis, gc[0], farbe->foreground());
    XDrawString(dis, vis, gc[0], 8, 8+asc, text, textlen);
  }

  // auf den Schirm!
  if (win && vis) {
    XCopyArea(dis, vis, win, gc[0], 0, 0, a.width, a.height, 0,0);
  }
}

// Circles, Arrows & Co.

void GraphicCircle::scale(double radius) {
  radius /= 2.0; 
  w = f->xx(radius); 
  h = f->yy(radius);
}

void GraphicCircle::draw(Drawable drw, Vector X, double v, int color) {
  cerr << "Sorry, under construction." << endl;
}


void GraphicArrow::draw(Drawable drw, Vector X, Vector V, int color) {
  double laxis = V.length()/vmax;
  laxis = (laxis<ratio ? ratio : laxis) * length;
  double laxis2 = laxis/2.0;
  double ex = V.x/V.length(), ey = V.y/V.length();
  double sx = -ey, sy = ex;
  double lw = laxis2-width;
  double w2 = width/2.0;
  double w3 = w2*0.3;
  double px = X.x + ex*lw, py = X.y + ey*lw;
  double tx = X.x - ex*laxis2, ty = X.y - ey*laxis2;

  const int npoints = 8; 
  const int npoly = npoints-1;
  XPoint p[npoints];

  p[0].x = f->xx(X.x + ex*laxis2); p[0].y = f->yy(X.y + ey*laxis2);
  p[1].x = f->xx(px + sx*w2);  p[1].y = f->yy(py + sy*w2);
  p[2].x = f->xx(px + sx*w3);  p[2].y = f->yy(py + sy*w3);
  p[3].x = f->xx(tx + sx*w3);  p[3].y = f->yy(ty + sy*w3);
  p[4].x = f->xx(tx - sx*w3);  p[4].y = f->yy(ty - sy*w3);
  p[5].x = f->xx(px - sx*w3);  p[5].y = f->yy(py - sy*w3);
  p[6].x = f->xx(px - sx*w2);  p[6].y = f->yy(py - sy*w2);
  p[7] = p[0];
  
  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
  XFillPolygon(f->dis, drw, f->gc[0], p, npoly, Nonconvex, CoordModeOrigin);
  //XSetForeground(f->dis, f->gc[0], f->farbe->foreground());
  //XDrawLines(f->dis, drw, f->gc[0], p, npoints, CoordModeOrigin);
}                         

// GROUND 

#ifdef _GROUND_HH_
void Ground::draw(SzeneFenster *f, Drawable drw) {
  int i;
  Window rootwin;
  int ix, iy, x, y;
  unsigned int width, height, bord, depth;
  Status status;
  
  if (!(grid && g)) return;

  status = XGetGeometry(f->dis, drw, &rootwin, &x, &y,
			&width, &height, &bord, &depth);
  
  /* int beg = grid->indx(Vector(f->range.x1, f->range.y1));
     int end = grid->indx(Vector(f->range.x2, f->range.y2));
     int dx = grid->indx(Vector(Dx.x, 0.0));
     int dy = grid->indx(Vector(0.0, Dx.y));
     int nx = (grid->indx(Vector(f->range.x2, f->range.y1)) - beg) / dx;
     int ny = (grid->indx(Vector(f->range.x1, f->range.y2)) - beg) / dy; */

// malen
  XSetForeground(f->dis,f->gc[0], f->farbe->background());
  XFillRectangle(f->dis, drw, f->gc[0], 0, 0, f->width, f->height);
 
  if (f->szflag.color) {
    f->farbe->limits(gmin, gmax, Rainbow /* Temperature */);
  }
  else {
    f->farbe->limits(gmin, gmax, Grayscale);
  } 
  double dx = Dx.x; 
  double dy = Dx.y;
  int w = int(f->pixpm * dx);
  while (w<5) {
    dx += Dx.x;
    w = int(f->pixpm * dx);
  }
  int w2 = w/2;
  
  int h = int(f->pixpm * dy);
  while (h<5) {
    dy += Dx.y;
    h =  int(f->pixpm * dy);
  }
  int h2 = h/2;
  w++; h++; 
    
  double val;
  Vector P;
  for (P.x=f->range.x1; P.x<=f->range.x2; P.x+=Dx.x) {
    for (P.y=f->range.y1; P.y<=f->range.y2; P.y+=Dx.y) {
      if (grid->inside(P, &i)) {
	val = g[i];
	XSetForeground(f->dis, f->gc[0], f->farbe->scale(val));
	XFillRectangle(f->dis,drw,f->gc[0],f->xx(P.x)-w2,f->yy(P.y)-h2,w,h);
      }
    }
  }
}
#endif

#ifdef _PEDESTRIAN_HH_
void Walker::draw(SzeneFenster *f, Drawable drw) {
  struct Floor *floor;
  int i, j, k, w;
  XPoint *pp = NULL; 
  int Npoints = Ntail+1;
  if (f->szflag.trace) {
    pp = new XPoint[Npoints];
  }
  for (w=0; w<popsize(); ++w) {
    double ratio = 1.0; // V.length()/f->vmax*0.7+0.3;;
    if (flag[w].visible) {
      int xwin =  f->xx(X[w].x);
      int ywin =  f->yy(X[w].y);
      XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));

      // Spur (trace) zeichenen 
      if (f->szflag.trace && pp) {
	int i, j;
	for (i=0; i<Ntail; ++i) {
	  j = (itail[w]+1+i)%Ntail;
	  pp[i].x = f->xx(Xtail[w][j].x);
	  pp[i].y = f->yy(Xtail[w][j].y);
	}
	pp[Ntail].x = f->xx(X[w].x);
	pp[Ntail].y = f->yy(X[w].y);
	//Spur muss zweimal gezeichnet werden 
	XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	XDrawLines(f->dis, drw, f->gc[0],
		   pp, Npoints, CoordModeOrigin); 
      }
      
      // Symbol zeichnen 
      
      unsigned int aa = int(width * f->pixpm);
      unsigned int bb = int(width * f->pixpm * ratio);
      unsigned int  a2 = aa/2;
      unsigned int  b2 = bb/2;
      
      switch (f->szflag.walker) {
      default:
      case 1: /* Circle */
	if (f->szflag.color) {
	  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	  XFillArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		   bb, bb, 0, 360*64); 
	}
	else {
	  XSetForeground(f->dis, f->gc[0], f->farbe->black());
	  if (color<4) 
	    XFillArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		     bb,bb, 0, 360*64); 
	  else 
	    XDrawArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		     bb,bb, 0, 360*64); 
	}
	test(printf("w: %lf|%lf, %d|%d, %d\n",X[w].x,X[w].y, xwin, ywin, bb));
	break;
      case 2:  /* Fixed Size */
	if (f->szflag.color) {
	  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	  XFillArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	}
	else {
	  XSetForeground(f->dis, f->gc[0], f->farbe->black());
	  if (color<4) 
	    XFillArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		     aa, aa, 0, 360*64); 
	  else 
	    XDrawArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		     aa, aa, 0, 360*64); 
	}
	break;
      case 4: // arrow
	f->symbol.arrow->draw(drw, X[w], V[w], color);
	break;
      case 6: // arrow
	XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	XDrawArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		 aa, aa, 0, 360*64); 
	f->symbol.arrow->draw(drw, X[w], V[w], color);
	break;
      } // switch   
    } // if visible
  } // for w
  return;
}
#endif

#ifdef _STREET_HH_
void Wall::draw(class SzeneFenster *f, Drawable drw) {  
  if (N<1) return;
  test(printf("ow: %lf|%lf, %d|%d\n", P.x, P.y, f->xx(P.x), f->yy(P.y)));
  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
  XPoint points[N+1];
  for (int i=0; i<N; ++i) {
    points[i].x = f->xx(PP[i].x);
    points[i].y = f->yy(PP[i].y);
  }
  points[N].x = f->xx((PP[N-1]+EEp[N-1]*aa[N-1]).x);
  points[N].y = f->yy((PP[N-1]+EEp[N-1]*aa[N-1]).y);
  XDrawLines(f->dis, drw, f->gc[0], points, N+1, CoordModeOrigin);
}
  
void CWall::draw(class SzeneFenster *f, Drawable drw) {
  test(printf("oc: %lf|%lf, %d|%d\n", P.x, P.y, f->xx(P.x), f->yy(P.y)));
  unsigned int a = (unsigned int) int(2.0*radius*f->pixpm);
  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
  XDrawArc(f->dis, drw, f->gc[0], f->xx(P.x-radius), f->yy(P.y-radius),
	   a, a, 0, 360*64);
}

void Decoration::draw(class SzeneFenster *f, Drawable drw) {
  int fc, c, i;
  switch (type) {
  case DecoPOLYGON:
    { 
      XPoint pp[N];
      for (i=0; i<N; ++i) {
	pp[i].x = f->xx(x[i]);
	pp[i].y = f->yy(y[i]);
      }
      if (f->szflag.color) {
	XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	XFillPolygon(f->dis, drw, f->gc[0], pp, N, Complex, CoordModeOrigin);
      }
      else {
	XSetForeground(f->dis, f->gc[0], f->farbe->foreground());
	XDrawLines(f->dis, drw, f->gc[0], pp, N, CoordModeOrigin);
      }
    }
    break;
  case DecoCIRCLE:
    unsigned int a = (unsigned int) int(2.0*radius*f->pixpm);
    if (f->szflag.color) {
      XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
      XDrawArc(f->dis, drw, f->gc[0], f->xx(*x-radius), f->yy(*y-radius),
	       a, a, 0, 360*64);
    }
    else {
      XSetForeground(f->dis, f->gc[0], f->farbe->foreground());
      XFillArc(f->dis, drw, f->gc[0], f->xx(*x-radius), f->yy(*y-radius),
	       a, a, 0, 360*64);
    }
    break;
  case DecoTEXT:
    break;
  }
}

void Street::draw(class SzeneFenster *f, Drawable drw) {
  announce("Street::draw\n");
  for (Pointer<Decoration> dec=decorations; dec; ++dec) {
    dec->draw(f, drw);
  }
  for(Pointer<Obstacle> o=obstacles; o; ++o) {
    announce("o");
    o->draw(f, drw);
  } 
  announce("\n\n");
}

void Gate::draw(class SzeneFenster *f, Drawable drw) {
  XSetForeground(f->dis, f->gc[1], f->farbe->pix(color));
#define punktpunktrepresentation
#ifdef punktpunktrepresentation
  XDrawLine(f->dis, drw, f->gc[1], f->xx(P.x), f->yy(P.y), 
	    f->xx(Q.x), f->yy(Q.y));
#else
  XDrawLine(f->dis, drw, f->gc[1], f->xx(P.x), f->yy(P.y), 
	    f->xx((P+Ep*a).x), f->yy((P+Ep*a).y));
#endif
}
  
void MultipleGate::draw(class SzeneFenster *f, Drawable drw) {
  for (Pointer<Gate> gat=gates; gat; ++gat) {
    gat->draw(f, drw);
  }
}

#endif


#ifdef _PEDESTRIAN_HH_
void WalkerPop::draw(class SzeneFenster *f, Drawable drw) {
  if (f->szflag.walker) {
    int i;
    switch (f->szflag.walker) {
    default:
    case 1: /* Circle */
      for (i=0; i<WN; ++i) {
	int xwin =  f->xx(W[i].pos.x);
	int ywin =  f->yy(W[i].pos.y);
	double ratio = 0.3 + 0.7 * (W[i].vel.length()/W[i].v0);
	int color = W[i].clan+2;
	unsigned int aa = int(f->pedwidth * f->pixpm);
	unsigned int bb = int(f->pedwidth * f->pixpm * ratio);
	unsigned int  a2 = aa/2;
	unsigned int  b2 = bb/2;
	
	if (f->szflag.color) {
	  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	  XFillArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		   bb, bb, 0, 360*64); 
	}
	else {
	  XSetForeground(f->dis, f->gc[0], f->farbe->black());
	  if (color<4) 
	    XFillArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		     bb,bb, 0, 360*64); 
	  else 
	    XDrawArc(f->dis, drw, f->gc[0], xwin - b2, ywin - b2,
		   bb,bb, 0, 360*64); 
	}
      }
      break;
    case 2:  /* Fixed Size */
      for (i=0; i<WN; ++i) {
	int xwin =  f->xx(W[i].pos.x);
	int ywin =  f->yy(W[i].pos.y);
	double ratio = 0.3 + 0.7 * (W[i].vel.length()/W[i].v0);
	int color = W[i].clan+2;
	unsigned int aa = int(f->pedwidth * f->pixpm);
	unsigned int bb = int(f->pedwidth * f->pixpm * ratio);
	unsigned int  a2 = aa/2;
	unsigned int  b2 = bb/2;
	
	if (f->szflag.color) {
	  XSetForeground(f->dis, f->gc[0], f->farbe->pix(color));
	  XFillArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	}
	else {
	  XSetForeground(f->dis, f->gc[0], f->farbe->black());
	if (color<4) 
	  XFillArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	else 
	  XDrawArc(f->dis, drw, f->gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	}
      }
      break;
    case 4: // shadowed, symbol
      for (i=0; i<WN; ++i) {
	f->symbol.arrow->draw(drw, W[i].pos, W[i].vel, W[i].clan+2);
      }
      break;
    } // switch   
  }
}
#endif


ReplayFenster::ReplayFenster() {
  W = NULL;
  WN = 0;
  Nmemo = 0;
  moviename = NULL;
  movie = -1;
}

ReplayFenster::~ReplayFenster() {
  if (W) delete W;
}
			       
void ReplayFenster::init() {
  SzeneFenster::init();
  get(parameter, "movie", &moviename, "movie.data");
  get(parameter, "pedwidth", &pedwidth, 0.7);
  movie = open(moviename, O_RDONLY, 0);
  T = 0.0;
}

int ReplayFenster::read() {
  return 0;
}

void ReplayFenster::draw() {
  Arg args[6];
  int n;
  unsigned long mypixel;
  char hi[] = "Hallo Peter!";
  bool _size, _scroll;
  if (!farbe) {
    farbe = new FarbPalette(dis);
  }
  if (!win) {
    printf("kein Window!\n"); return;
  }
  if (!gc[0]) {
    XGCValues xgcv;
    XSetWindowAttributes attrs;
    attrs.bit_gravity = ForgetGravity;
    XChangeWindowAttributes(dis, XtWindow(area), CWBitGravity, &attrs);
    xgcv.foreground = farbe->foreground();
    xgcv.background = farbe->background();
    xgcv.line_width = linewidth;
    xgcv.line_style = LineSolid;
    gc[0] = XCreateGC(dis, win, GCmyMask, &xgcv);
    xgcv.line_style = LineOnOffDash;
    gc[1] = XCreateGC(dis, win, GCmyMask, &xgcv);
    Ngc = 2;
  }
  XGetWindowAttributes(dis, win, &a);
  // test(cout << "attributes\n"); 
  /* Fensterinhalt rekonstruieren, evtl. abh. von d. Einstellung */
  //!!!!redraw();
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
  if (_size || _scroll || szflag.ground) {
    // neuen Hintergrund
    old.width = a.width; old.height = a.height; old.px = px; old.py = py;
    if (_size) {
      if (pim) XFreePixmap(dis, pim);
      if (vis) XFreePixmap(dis, vis);
      pim = XCreatePixmap(dis, win, a.width, a.height, a.depth);
      vis = XCreatePixmap(dis, win, a.width, a.height, a.depth);
    } 
    
#undef STREIFIGERHINTERGRUND
#ifdef STREIFIGERHINTERGRUND
    for (int ii=0; ii<8; ii++) {
      XSetForeground(dis, gc[0], farbe->pix(ii)); 
      XFillRectangle(dis, pim, gc[0], ii*a.width/8, 0,
		     a.width/8, a.height);
    }
#else // normal
    mypixel = farbe->background();
    XSetForeground(dis, gc[0], mypixel); 
    XFillRectangle(dis, pim, gc[0], 0, 0, a.width, a.height);
#endif
    
    if (szflag.ground && ground) { //Ground 
      ground->draw(this, pim);
    }
    
#ifdef _STREET_HH_
    if (szflag.obstac && floor) { //Street
      for (Pointer<Street> street=floor->streets; street; ++street) {
	street->draw(this, pim); // alle Strassen Malen
      }
      for (Pointer<Gate> gat=floor->gates; gat; ++gat) {
	gat->draw(this, pim); // alle Tore
      } 
    }
#endif
  } // ende hintergrund
  
  XCopyArea(dis, pim, vis, gc[0], 0, 0, a.width, a.height, 0, 0);    
  //peds
#ifdef _PEDESTRIAN_HH_
  if (szflag.walker) {
    for (int i=0; i<WN; ++i) {
      // draw walker in vis 
      // cout << i << ':' << W[i].pos.x << '|' << W[i].pos.y << ' ' << W[i].vel.length() << ' ' << W[i].clan << endl;
      int xwin =  xx(W[i].pos.x);
      int ywin =  yy(W[i].pos.y);
      double ratio = 0.3 + 0.7 * (W[i].vel.length()/W[i].v0);
      int color = W[i].clan;
      unsigned int aa = int(pedwidth * pixpm);
      unsigned int bb = int(pedwidth * pixpm * ratio);
      unsigned int  a2 = aa/2;
      unsigned int  b2 = bb/2;
      
      switch (szflag.walker) {
      default:
      case 1: /* Circle */
	if (szflag.color) {
	  XSetForeground(dis, gc[0], farbe->pix(color));
	  XFillArc(dis, vis, gc[0], xwin - b2, ywin - b2,
		   bb, bb, 0, 360*64); 
	}
	else {
	  XSetForeground(dis, gc[0], farbe->black());
	  if (color<4) 
	    XFillArc(dis, vis, gc[0], xwin - b2, ywin - b2,
		     bb,bb, 0, 360*64); 
	  else 
	    XDrawArc(dis, vis, gc[0], xwin - b2, ywin - b2,
		   bb,bb, 0, 360*64); 
	}
	break;
      case 2:  /* Fixed Size */
	if (szflag.color) {
	  XSetForeground(dis, gc[0], farbe->pix(color));
	  XFillArc(dis, vis, gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	}
	else {
	  XSetForeground(dis, gc[0], farbe->black());
	if (color<4) 
	  XFillArc(dis, vis, gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	else 
	  XDrawArc(dis, vis, gc[0], xwin - a2, ywin - a2,
		   aa, aa, 0, 360*64); 
	}
      break;
    case 4: // shadowed, symbol
	int j, k, x, y, a, b;
	j = int(W[i].v0/vmax*4);
	if (j<0) j = 0;
	if (j>3) j = 3; 
	k = int(W[i].vel.length()/W[i].v0*4);
	if (k<0) k = 0;
	if (k>3) k = 3;
	//a = symb_width(color, j, k);
	//b = symb_height(color, j, k); 
	x = xwin - a/2;
	y = ywin - b/2;
	// XCopyArea(f->dis, 
	//	symb_pim(color, j, k), drw, f->gc[0], 0, 0, a, b, x, y);
	break;
      } // switch   
    } // for
  }
#endif 
  if (win && vis) {
    XCopyArea(dis, vis, win, gc[0], 0, 0, a.width, a.height, 0,0);
  }
  char text[20];
  sprintf(text, "time %5.1lf", T);
  //  XmTextSetString(message, text);
}



/* end of file */












