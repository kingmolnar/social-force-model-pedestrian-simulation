#define NTEST
#include "test.h"
#include <agX/agxgra.h>
#include <agX/agxaxi.h>
#include <agX/agxcon.h>
#include "plot.hh"
#include <stdlib.h>

#include "floor.hh"
#include "street.hh"
#include "pedestrian.hh"
#include "ground.hh"
#include "movie.hh"

extern "C" {
  int XuValueToColor(double);
}

struct {
  double length;
  double width;
  double ratio;
  double framewidth;
  double vmax;
  double color;
} ArrowParameters;

void ArrowScale(double l, double w, double r, double max) {
  ArrowParameters.length = l;
  ArrowParameters.ratio = r;
  ArrowParameters.width = w;
  ArrowParameters.vmax = max;
}

void ArrowFrameWidth(double f) {ArrowParameters.framewidth = f;}

void ArrowDraw(Vector X, Vector V, int color_index) {
  const int num_points = 7; 
  float x[num_points];
  float y[num_points];
  float r = ArrowParameters.ratio;
  float laxis = V.length()/ArrowParameters.vmax;
  laxis = (laxis<ArrowParameters.ratio ? ArrowParameters.ratio : laxis)
    * ArrowParameters.length;
  float laxis2 = laxis/2.0;
  float ex = V.x/V.length();
  float ey = V.y/V.length();
  float sx = -ey;
  float sy = ex;
  float lw = laxis2-ArrowParameters.width;
  float w2 = ArrowParameters.width/2.0;
  float w3 = w2*0.3;
  float px = X.x + ex*lw;
  float py = X.y + ey*lw;
  float tx = X.x - ex*laxis2;
  float ty = X.y - ey*laxis2;
  
  x[0] = X.x + ex*laxis2;
  y[0] = X.y + ey*laxis2;
  x[1] = px + sx*w2;
  y[1] = py + sy*w2;
  x[2] = px + sx*w3;
  y[2] = py + sy*w3;
  x[3] = tx + sx*w3;
  y[3] = ty + sy*w3;
  x[4] = tx - sx*w3;
  y[4] = ty - sy*w3;
  x[5] = px - sx*w3;
  y[5] = py - sy*w3;
  x[6] = px - sx*w2;
  y[6] = py - sy*w2;
  
  // void XuPolygonDraw (float x_array[num_points], float y_array[num_points],
  //                     int num_points, int color_index, double frame_width)
  XuPolygonDraw (x, y, num_points, color_index, ArrowParameters.framewidth);
}                         




void Plot::open() {
  /*   XuDeviceSelect(plot->device->command); */
  char *comm = new char[32];
  if (!plotisopen) {
    plotisopen = 1;
    plotisrunning = 0;
    parameter->get("command", &comm);
    XuDeviceSelect(comm);
    XuOpen(XuCREATE);
    XuColorDefaultLoad(32);
    XuDrawingAreaQuery(&width_mm, &height_mm, &width_pix, &height_pix);
  }
}

void Plot::close() {
  if (plotisopen && !plotisrunning) {
    plotisopen = 0;
    XuClose();
    if (shell && *shell) system(shell);
    announce("plot done.\n");
  }
}

SimulObject *newSceneryPlot() {
  return new SceneryPlot;
}

SceneryPlot::SceneryPlot() {
  floor = NULL;
  ground = NULL;
  ParameterDef d[] = { {"style", "1"},
		       {"title", "'"},
		       {"shell", ""},
		       {"comment", "'"},
		       {"command", "'select HPOSTA4; exit"}
		     };
  if (!parameter) parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  if (parameter->b) delete parameter->b;
  intflag = 0;
  flag.color = 1;
  flag.rangefull = 1;
  flag.backgroundoff = 1;
  flag.trace = 0;
  flag.obstaclestyle = 2;
  flag.walkerstyle = 1;
  //  parameter->flags = intflag;
  plotisopen = 0;
  plotisrunning = 0;
  //  objecttype = _PLOT;
}

void SceneryPlot::init() {
  Plot::init();
  //int fl = parameter->flags;
  //  get(parameter, "flag", &intflag, intflag);
  int tmp;
  get(parameter, "color", &tmp, flag.color); 
  flag.color = tmp ? 1: 0;
  get(parameter, "fullrange", &tmp, flag.rangefull);
  flag.rangefull = tmp ? 1: 0;
  get(parameter, "nobackground", &tmp, flag.backgroundoff);
  flag.backgroundoff = tmp ? 1: 0;
  get(parameter, "trace", &tmp, flag.trace);
  flag.trace = tmp ? 1: 0;
  get(parameter, "obstaclestyle", &tmp, flag.obstaclestyle);
  flag.obstaclestyle = tmp;
  get(parameter, "walkerstyle", &tmp, flag.walkerstyle);
  flag.walkerstyle = tmp;
  get(parameter, "axis", &tmp, flag.axis);
  flag.axis = tmp ? 1: 0;
  get(parameter, "axisstyle", &tmp, flag.axisstyle);
  flag.axisstyle = tmp;

  parameter->get("command", &comm);
  parameter->get("shell", &shell);
  char *titlestring;
  get(parameter, "title", &titlestring, "");
  for (int ci=0; ci<8; ++ci) {
    char tmp[7];
    sprintf(tmp, "color%1d", ci);
    get(parameter, tmp, colorindex+ci, ci+2);
  }
  //if (flag.rangefull==0) {}else {
#if defined _FLOOR_HH_ && defined _STREET_HH_ // && defined _PEDESTRIAN_HH_
  if (floor) { 
    xmin = floor->Xmin.x;
    xmax = floor->Xmax.x;
    ymin = floor->Xmin.y;
    ymax = floor->Xmax.y;
  }
  else if (streets.N()) {
    int n = 0;
    for (Pointer<Street> str = streets; str; ++str) {
      if (n==0 || str->Xmin.x < xmin) xmin = str->Xmin.x;
      if (n==0 || str->Xmin.y < ymin) ymin = str->Xmin.y;
      if (n==0 || str->Xmax.x > xmax) xmax = str->Xmax.x;
      if (n==0 || str->Xmax.y > ymax) ymax = str->Xmax.y;
      ++n;
    }
  }
#else 
  xmin = 0.0; xmax = 20.0; ymin = 0.0; ymax = 15.0;
#endif
  /* definierte Grenzen */
  get(parameter, "xmin", &xmin, xmin);
  get(parameter, "ymin", &ymin, ymin);
  get(parameter, "xmax", &xmax, xmax);
  get(parameter, "ymax", &ymax, ymax);
  test(printf ("range %lf %lf %lf %lf \n", xmin, xmax, ymin, ymax));
  //else {// Error("invalid range.")}
  
  double vmax = floor? floor->vmax: 1.5;
  get(parameter, "vmax", &pedvmax, vmax);
  get(parameter, "pedwidth", &pedwidth, 0.7);
  get(parameter, "arrowlength", &arrowlength, 1.0);
  get(parameter, "arrowwidth", &arrowwidth, 0.7);
  get(parameter, "arrowratio", &arrowratio, 0.3);
  get(parameter, "framewidth", &framewidth, 0.15);
  get(parameter, "linewidth", &linewidth, 0.15);
  get(parameter, "duration", &duration, 0.0);
  get(parameter, "interval", &interval, 0.0);
  plotisopen = 0;
  plotisrunning = 0;
  objecttask = SET_VALUES;
}

int SceneryPlot::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  switch (type) {
#ifdef _FLOOR_HH_
  case _FLOOR:
    floor = (Floor*)obj;
    good = 1;
    break;
#endif
#ifdef _STREET_HH_
  case _STREET:
    streets.append(obj);
    good = 1;
    break;
#endif
#ifdef _GROUND_HH_
  case _GROUND:
    ground = (Ground*)obj;
    good = 1;
    break;
#endif
#ifdef _MOVIE_HH_
  case _MOVIE:
    movie = (Movie*)obj;
    good = 1;
    break;
#endif
  default:
    good = 0;
    break;
  }
  return good ? 1 : Plot::ref(obj, type);
}


void SceneryPlot::plot() {
  char cc[32];
  char *comm = cc;
  parameter->get("command", &comm);
  XuDeviceSelect(comm);

  XuOpen(XuCREATE);
  XuColorDefaultLoad(32);
  XuDrawingAreaQuery(&width_mm, &height_mm, &width_pix, &height_pix);

  // Format
  int format = ((ymax-ymin)>(xmax-xmin)?1:0) + (width_mm>height_mm?2:0) + 1;
  switch (format) {
  case 1: // quer auf hoch
    xsize = height_mm;
    xsize *= 0.9;
    ysize = (ymax-ymin)/(xmax-xmin)*xsize;
    break;
  case 2: // hoch auf hoch
    ysize = height_mm;
    ysize *= 0.9;
    xsize = (xmax-xmin)/(ymax-ymin)*ysize;
    break;
  case 3: // quer auf quer
    xsize = width_mm;
    xsize *= 0.9;
    ysize = (ymax-ymin)/(xmax-xmin)*xsize;
    break;
  case 4: // hoch auf quer 
    ysize = width_mm;
    ysize *= 0.9;
    xsize = (xmax-xmin)/(ymax-ymin)*ysize;
    break;
  }
  
  xoff = 0.5*(width_mm-xsize);
  yoff = 0.5*(height_mm-ysize);
  
  XuViewport(xoff, yoff, xsize, ysize);
  XuViewWorldLimits(xmin, xmax, ymax, ymin, 0, 0);
  XuClip(XuON);
  XuViewWorkbox (fabs(xmax-xmin), fabs(ymax-ymin), 0.); 
  
  // Hintergrund 
  if (!flag.backgroundoff) {
#ifdef _GROUND_HH_
    if (ground) {
      ground->hardcopy(this);
      announce("Ground Hardcopy\n");
    }
#ifdef _MOVIE_HH_
    else if (movie) {
      Pointer<Ground> gro = movie->grounds;
      gro->hardcopy(this);
    }
#endif // _MOVIE_HH_
#ifdef _FLOOR_HH_
    else if (floor) {
      Pointer<Ground> gro = floor->grounds;
      gro->hardcopy(this);
    }
#endif // _FLOOR_HH_
#endif
  }
  
  // Axen 
  if (flag.axis) {
    float height = 2.0;
      XuClip (XuOFF);
    XuViewWorld();
    XuAxisTickline (XuTICKLINES, XuI_IGNORE, XuF_IGNORE, XuI_IGNORE);
    XuAxisDrawOptions (XuTICKLINES, XuON, XuOFF);  
    XuAxisDraw(XuX_AXIS, ymin, height, XuREVERSED);
    XuAxisDraw(XuY_AXIS, xmin, height, XuNORMAL);
  }
  
  // Strassen
#if defined _FLOOR_HH_ && defined _STREET_HH_ 
  Pointer<Street> str;
  if (floor) str = floor->streets;
  else str = streets;
  for (; str; ++str) {
    if (flag.obstaclestyle < 4) {
      str->hardcopy(this);
#ifdef _ATTRACTION_HH_
      for (att=str->attractions; att; ++att) {
	att->hardcopy(this);
      }  
#endif
    }
  }
#endif
  
  // def Arrows 
  ArrowScale(arrowlength, arrowwidth, arrowratio, pedvmax);
#if defined _FLOOR_HH_ && defined _STREET_HH_ && defined _PEDESTRIAN_HH_
  Pointer<Pedestrian> ped;
  if (flag.trace) { // trace
    // #ifdef _MOVIE_HH_
    //     if (movie && movie->subpops.N()) {
    //       for (Pointer<WalkerPop> wop=movie->subpops; wop; ++wop) {
    // 	wop->trace(this);
    //       }
    //     }
    // #else
    //     if (0);
    // #endif
    if (floor && floor->pedestrians.N()) {
      for (Pointer<Pedestrian> ped=floor->pedestrians; ped; ++ped) {
	ped->trace(this); 
      }
    }
    else if (floor) {
      for (Pointer<Street> street=floor->streets; street; ++street) {
	for (Pointer<Pedestrian> ped=street->pedestrians; ped; ++ped) {
	  ped->trace(this); 
	}
      }
    } 
  }
  
  if (flag.walkerstyle) { // hardcopy
    if (floor && floor->pedestrians.N()) {
      for (Pointer<Pedestrian> ped=floor->pedestrians; ped; ++ped) {
	ped->hardcopy(this); 
      }
    }
    else if (floor) {
      for (Pointer<Street> street=floor->streets; street; ++street) {
	for (Pointer<Pedestrian> ped=street->pedestrians; ped; ++ped) {
	  ped->hardcopy(this); 
	}
      }
    } 
  }
#endif
  XuClose();
  if (shell && *shell) system(shell);
  announce("plot done.\n");
  return;
}




#ifndef PlotSHADOW
#define PlotSHADOW 1 
#endif

#ifdef _PEDESTRIAN_HH_

void Walker::hardcopy(SceneryPlot *plot) {
  Floor *floor;
  double x, y, r, fr, a, b;
  Vector Xprime;
  int plflag;
  int sty = 0;
  int c = plot->flag.color ? 
    plot->colorindex[clan%8] : (clan<4 ? 1 : 0); 
  
  int w;
  for (w=0; w<popsize(); ++w) {
    if (flag[w].visible) {
      XuViewWorld();
      switch (plot->flag.walkerstyle) {
      default:
      case 1: /* Circle */
	a = (0.3 + V[w].length()/plot->pedvmax * 0.7) * width;
	if (plot->flag.color) { // farbe 
	  XuCircleDraw(X[w].x, X[w].y, -a/2.0, c, 0.0);
	}
	else { // schwarz-weiss
	  //c =  (color<4 ? 1 : 0);
	  XuCircleDraw(X[w].x, X[w].y, -a/2.0, c, plot->framewidth);
	}
	break;
      case 2: /* shadowed circle, fixed circle */
	a = width;
	if (plot->flag.color) { // farbe 
	  XuCircleDraw(X[w].x, X[w].y, -a/2.0, c, 0.0);
	}
	else { // schwarz-weiss
	  XuCircleDraw(X[w].x, X[w].y, -a/2.0, c, plot->framewidth);
	}
	break;
      case 4: /* arrow */ 
	if (plot->flag.color) {
	  ArrowFrameWidth(0.0);
	  ArrowDraw(X[w], V[w], c);
	}
	else {
	  ArrowFrameWidth(plot->framewidth);
	  ArrowDraw(X[w], V[w], c);
	}
	break;
      } // switch 
    } // visible
  } // for w
}

void Walker::trace(SceneryPlot *plot) {
  //int c = plot->flag.color ? 
  int c = plot->colorindex[clan%8];
  XuWidthColor(plot->framewidth, c);
  int i, j;
  int Npoints = Ntail+1;
  float *x = new float[Npoints];
  float *y = new float[Npoints];
  int w;
  for (w=0; w<popsize(); ++w) {
    if (flag[w].visible) {
      for (i=0; i<Ntail; ++i) {
	j = (itail[w]+1+i)%Ntail;
	x[i] = Xtail[w][j].x;
	y[i] = Xtail[w][j].y;
      }
      x[Ntail] = X[w].x;
      y[Ntail] = X[w].y;
      
      //if (plot->flag.color) {
      // XuLineStyle(1);
      //} else { XuWidthColor(plot->framewidth, 1); } 
      XuLineDraw (x, y, Npoints);
    }
  } // for w
  delete x;
  delete y;
}

void WalkerPop::hardcopy(SceneryPlot *plot) {
  Floor *floor;
  double x, y, r, fr, a, b;
  Vector Xprime;
  int c, plflag;
  int sty = 0; 
  int i;
  parameter->get("style", &sty);
  double pedwidth;
  get(parameter, "pedwidth", &pedwidth, 0.7);
  
  // if (floor) vmax = floor->vmax;
  XuViewWorld();
  switch (plot->flag.walkerstyle) {
  default:
  case 1: /* Circle */
    for (i=0; i<WN; ++i) {
      //b = pedwidth * (0.3 + v0 / vmax * 0.7); 
      a = (0.3 + W[i].vel.length()/plot->pedvmax * 0.7) * pedwidth;
      c = plot->flag.color?plot->colorindex[W[i].clan%8]:(W[i].clan<4 ? 1 : 0);
      if (plot->flag.color) { // farbe 
	XuCircleDraw(W[i].pos.x, W[i].pos.y, -a/2.0, c, 0.0);
      }
      else { // schwarz-weiss
	XuCircleDraw(W[i].pos.x, W[i].pos.y, -a/2.0, c, plot->framewidth);
      }
    }
    break;
  case 2: /* shadowed circle, fixed circle */
    for (i=0; i<WN; ++i) {
      c = plot->flag.color?plot->colorindex[W[i].clan%8]:(W[i].clan<4 ? 1 : 0);
      if (plot->flag.color) { // farbe 
	XuCircleDraw(W[i].pos.x, W[i].pos.y, -a/2.0, c, 0.0);
      }
      else { // schwarz-weiss
	XuCircleDraw(W[i].pos.x, W[i].pos.y, -a/2.0, c, plot->framewidth);
      }
    }
    break;
    //  plflag |= PlotSHADOW;
  case 4: /* ellipse */
      /* XuEllipseSize (-a/2.0, 1.0);
	 XuEllipseLimits (0.0, 0.0);
	 XuEllipseWidthColor(c, fr);
	 XuEllipseDraw (XuFILLED_ARC, x, y); */     
      break;
  case 8: /* arrow */
    if (plot->flag.color) {
      ArrowFrameWidth(0.0);
      for (i=0; i<WN; ++i) {
	c = plot->flag.color?
	  plot->colorindex[W[i].clan%8]:(W[i].clan<4 ? 1 : 0);
	ArrowDraw(W[i].pos, W[i].vel, c);
      }
    }
    else {
      ArrowFrameWidth(plot->framewidth);
      for (i=0; i<WN; ++i) {
	c = W[i].clan<4 ? 1 : 0;
	ArrowDraw(W[i].pos, W[i].vel, c);
      }
    }
    break;
  }
}

void WalkerPop::trace(SceneryPlot *plot) {
}


#endif 



#ifdef _STREET_HH_
void Street::hardcopy(SceneryPlot *plot) {
  if (plot->flag.obstaclestyle>=2) {
    for (Pointer<Decoration> dec=decorations; dec; ++dec) {
      dec->hardcopy(plot);
    }
  }
  for (Pointer<Obstacle> obs=obstacles; obs; ++obs) {
    obs->hardcopy(plot);
  }
}
  
void Wall::hardcopy(SceneryPlot *plot) {
  float xa[N+1], ya[N+1];
  int c = (plot->flag.color) ? color : 1;
  for (int i=0; i<N; ++i) {
    xa[i] = PP[i].x; 
    ya[i] = PP[i].y;
  }
  xa[N+1] = (PP[N-1]+EEp[N-1]*aa[N-1]).x;
  ya[N+1] = (PP[N-1]+EEp[N-1]*aa[N-1]).y;
  XuViewWorld();
  XuWidthColor(plot->linewidth, c);
  XuLineDraw (xa, ya, N+1);                                   
}

void CWall::hardcopy(SceneryPlot *plot) {
  int c = (plot->flag.color) ? color : 1;
  XuViewWorld();
  double rmm = XuWorkboxToMM(radius);
  XuEllipseSize(rmm, 1.0);
  XuEllipseLimits (0.0, 0.0);
  XuEllipseWidthColor(c, plot->linewidth);
  switch (plot->flag.obstaclestyle) {
  case 1:   
    XuEllipseDraw(XuHOLLOW_ARC, P.x, P.y);
    break;
  case 2:
    XuEllipseDraw(XuFILLED_ARC, P.x, P.y);      
    break;
  }
}


void Decoration::hardcopy(SceneryPlot *plot) {
  int fc, c;
  if (plot->flag.color) {
    fc = framecolor;
    c = color;
  }
  else {
    fc = framecolor ? 1 : 0;
    c = color; // c = color ? 4 : 0;
  }
  XuPolygonFrameColor (fc);   
  XuViewWorld();
  switch (type) {
  case DecoPOLYGON:
    float *xx = new float[N];
    float *yy = new float[N];
    for (int i=0; i<N; ++i) {
      xx[i] = x[i];
      yy[i] = y[i];
    }
    XuPolygonDraw (xx, yy, N, c, framewidth);
    delete xx;
    delete yy;
    break;
  case DecoCIRCLE:
    double xcirc = *x;
    double ycirc = *y;
    XuCircleDraw (xcirc, ycirc, radius, c, framewidth);
    break;
  case DecoTEXT:
    break;
  }
}

void Gate::hardcopy(SceneryPlot *plot) {
}

void MultipleGate::hardcopy(class SceneryPlot *pl) {
  for (Pointer<Gate> gat=gates; gat; ++gat) {
    gat->hardcopy(pl);
  }
}
#endif /* STREET */

#ifdef _GROUND_HH_
void Ground::hardcopy(SceneryPlot *plot) {
  // definiere Farben im HLS-Modus
  int Ncolor = 11, Offset = 30;
  float color_array[3] = { 0.0, 50.0, 100.0 }; 
  float hatch_array[5];
  double dz = 1.0/Ncolor;
  for (int c=0; c<Ncolor; ++c) {
    color_array[0] = 360.0-240*dz*double(c);
    //color_array[1] = 50.0;     color_array[2] = 100.0;
    XuColor (XuCOLOR , Offset+c, color_array, hatch_array);
  }
  XuShadingColorTable (Offset, Ncolor);                              
  XuClassesMinMax (gmin, gmax, Ncolor-1);
  XuViewWorld();
  XuClip(XuON);
  Vector P;
  int i, clr;
  for (P.x=Xmin.x; P.x<=Xmax.x; P.x+=Dx.x) {
    for (P.y=Xmin.y; P.y<=Xmax.y; P.y+=Dx.y) {
      if (grid->inside(P, &i)) {
        clr = XuValueToColor (g[i]);
        XuRectangleDraw (P.x-Dx.x, P.y-Dx.y, P.x+Dx.x, P.y+Dx.y, clr, 0.0);
      }
    }
  }
}
#endif


// // header
// class ConturPlot: public Plot {
//   Floor *floor;
//   List streets;
//   Ground *ground;
//   float xmin, xmax, ymin, ymax, zmin, zmax, *z, zz;
//   struct {
//     unsigned int color : 1;
//     unsigned int range : 1;
//   } flag;  
// public:
//   ConturPlot();
//   void init();
//   int ref(SimulObject*);
//   void plot();
// }; 

// class GateStatPlot: public Plot {
// public:
//   GateStatPlot(Parameter*, SimulObject*);
//   void plot();
// }; 

SimulObject *newConturPlot() {
  return new ConturPlot;
}

ConturPlot::ConturPlot() {
  ParameterDef d[] = { {"xmin", ""}, {"xmax", ""},
		       {"ymin", ""}, {"ymax", ""},
		       {"device", ""},
		       {"style", "1"},
		       {"title", ""},
		       {"comment", ""},
		       {"command", "'select HPOSTA4; exit"}
		     };
  if (!parameter) parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  //  if (parameter->b) delete parameter->b;
  //  parameter->b = new bool[16];
}

void ConturPlot::init() {
  parameter->get("command", &comm);
  //  flag.color = parameter->b[1];
}

int ConturPlot::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  switch (type) {
#ifdef _FLOOR_HH_
  case _FLOOR:
    floor = (Floor*)obj;
    good = 1;
    break;
#endif
#ifdef _STREET_HH_
  case _STREET:
    //streets.append(obj);
    street = (Street*)obj;
    good = 1;
    break;
#endif
#ifdef _GROUND_HH_
  case _GROUND:
    ground = (Ground*)obj;
    good = 1;
    break;
#endif
#ifdef _PEDESTRIAN_HH_
  case _PEDESTRIAN:
    pedestrians.append(obj);
    good = 1;
    break;
#endif
  default:
    good = 0;
    break;
  }
  return good;
}

void ConturPlot::plot() {
  float xmin, xmax, ymin, ymax, zmin, zmax, *z, zz;
  int i, ix, iy, nx, ny, ncl;
  /* static char txtarr[4][MXLABEL] = {"","","",""}; */
  float tpx, tpy;
  float height, step;
  double x, y;
  Street* str;  
  if (ground) { 
#ifdef _GROUND_HH_
#endif
  }  
  else if (floor) {
    // str = floor->streets;
  }
  else {
    str = street;
  }
  //  for (;str;++str) 
  xmin = str->Xmin.x;
  xmax = str->Xmax.x;
  ymin = str->Xmin.y;
  ymax = str->Xmax.y;
  double dx = str->Dx.x;
  double dy = str->Dx.y;
  // berechne z
  nx = int((xmax - xmin)/dx + 1.0);
  ny = int((ymax - ymin)/dy + 1.0);
  
  z = new float[nx*ny];
  zmin = zmax = street->U(Vector(xmin,ymin)); 
  for (ix=0; ix<nx; ix++) {
    for (iy=0; iy<ny; iy++) {
      x = xmin+ix*dx;
      y = ymin+iy*dy;
      zz = z[ix+iy*nx] = str->U(Vector(x,y));
      if (zz<zmin) zmin = zz;
      if (zz>zmax) zmax = zz;
    }
  }
  if (!z && zmax<zmin) {
    printf("abbruch\n");
    return;
  }
  // Papierformat
  if ((ymax-ymin)<(xmax-xmin)) {  /* X ist laenger */
    xsize = width_mm>height_mm ? width_mm : height_mm;
    xsize *= 0.9;
    ysize = (ymax-ymin)/(xmax-xmin)*xsize;
  }
  else { /* Y ist laenger */
    ysize = width_mm>height_mm ? width_mm : height_mm;
    ysize *= 0.9;
    xsize = (xmax-xmin)/(ymax-ymin)*ysize;
  }
  
  xoff = 0.5*(width_mm-xsize);
  yoff = 0.5*(height_mm-ysize);
  
  XuViewWorldLimits(xmin, xmax, ymax, ymin, 0, 0);
  XuViewport(xoff, yoff, xsize, ysize);
  XuClip(XuON);
  /* XuViewWorkbox (1., 1., 0.); */ 
  
  //Contour Zeug
  parameter->get("ncl", &ncl);
  step = (zmax-zmin)/ncl;
  
  /* height = 0.012 * (xsize<ysize?xsize:ysize); */
  XuClassesStartStep (zmin, step, ncl);
  /* XuIsolineWidths (wi, NCL);
     XuIsolineLabel (0.0, 0, (-0.8 * xsize), XuOVERLAP_CHECK);
     for (i = 0; i < NCL; i++)
     hi[i] *= height;
     XuIsolineLabelHeight (hi, NCL);
     XuIsolineStyles (istyle, NCL);
     XuIsolineLabelBox (0, XuON);
     XuTextFont ("SWIM", XuSOFT);
     */
  
  parameter->get("style", &sty);
  switch(sty) {
  default:
  case 1:
    XuIsolineDraw(z, ny, nx);
    break;
  case 2:
    XuContourDraw (z, ny, nx);
    break;
  case 3:
    XuContourDraw (z, ny, nx);
    XuIsolineDraw(z, ny, nx);
    break;
  }	
  /* XuViewWorld ();
     XuTextFont ("ITAL", XuSOFT);
     XuTextHeight (height);
     for (i = 0; i < NP; i++)
     {
     XuTextJustification (XuCENTER, XuFONT_HALF);
     XuTextDraw ("*", xp[i], yp[i]);
     XuTextJustification (XuLEFT, XuFONT_HALF);
     XuTextFloatDraw (zp[i], 0, (xp[i] + 0.01 * (xmax - xmin)), yp[i]);
     } */
  
  /*      XuAxisFont (XuLABELS, "SWIM", XuSOFT);
	  XuAxisLabelFormat (0, 0, IUNDEF, IUNDEF);
	  XuAxisTickLayout (IUNDEF, RUNDEF, RUNDEF, dbl);
	  XuAxisMinorTickCount (ntick);
	  XuAxisDrawOptions (XuMINOR_TICKS, XuON, IUNDEF);
	  XuAxisSystemDraw (ymin, xmin, height, txtarr, MXLABEL);
	  XuAxisSystemDraw (ymax, xmax, height, txtarr, MXLABEL); */
  
  tpy = ymin - 0.1 * (ymax - ymin);
  tpx = 0.5 * (xmin + xmax);
  XuTextFont ("SWIM", XuSOFT);
  XuTextJustification (XuCENTER, XuFONT_CAP);
  XuTextHeight (3.0);
  XuTextDraw ("Text", tpx, tpy);
}; 

// GateStatPlot::GateStatPlot(Parameter *p, SimulObject *obj) {
// //  parameter = p;
// //  simobj = obj;
// }

// GateStatPlot::plot() {
// }; 



