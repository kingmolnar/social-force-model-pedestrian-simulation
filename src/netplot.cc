#include <agX/agxgra.h>
#include "netplot.hh"
#include "network.hh"

NetPlot::NetPlot(Parameter* p, Network* net) {
  if (!p || !net) return;
  
  parameter = p;
  char *comm;
  if (parameter->get("command", &comm)) XuDeviceSelect(comm);
  else XuDeviceSelect("");
  XuOpen(XuCREATE);
  XuColorDefaultLoad(32);

  float width_mm, height_mm;
  int width_pix, height_pix;

  XuDrawingAreaQuery(&width_mm, &height_mm, &width_pix, &height_pix);

  double xmin, xmax, ymin, ymax, xoff, yoff, xsize, ysize;
  char t[80]; 

  if (parameter->flag(4)) {
    /* definierte Grenzen */
    parameter->get("xmin", &xmin);
    parameter->get("ymin", &ymin);
    parameter->get("xmax", &xmax);
    parameter->get("ymax", &ymax);
  }
  else {
    Vector D = net->Xmax - net->Xmin;
    xmin = net->Xmin.x - 0.05 * D.x;
    xmax = net->Xmax.x + 0.05 * D.x;
    ymin = net->Xmin.y - 0.05 * D.y;
    ymax = net->Xmax.y + 0.05 * D.y;
  }
  
  /*   double a = xmax-xmin;
       double b = ymax-ymin;
       double ab = a/b;
       double wh = width_mm/height_mm;
       // kuerzere Kante, damit alles draufpasst
       if (a>b) { // horizontal
       if (ab>wh) {
       xsize = width_mm; // >height_mm ? width_mm : height_mm;
       xsize *= 0.9;
       ysize = xsize/ab;
       }
       else {
       ysize = height_mm;
       ysize *= 0.9;
       xsize = ysize*ab;
       }
    xoff = 0.5*(width_mm-xsize);
    yoff = 0.5*(height_mm-ysize);
    }
    else { // vertical
    if (ab>wh) {
    xsize = height_mm; // >height_mm ? width_mm : height_mm;
    xsize *= 0.9;
    ysize = xsize/ab;
    }
    else {
      ysize = width_mm;
      ysize *= 0.9;
      xsize = ysize*ab;
      }
      yoff = 0.5*(width_mm-xsize);
      xoff = 0.5*(height_mm-ysize);
      } */
  
  xsize = 0.9 * width_mm;
  ysize = 0.9 * height_mm; 
  xoff = 0.5*(width_mm-xsize);
  yoff = 0.5*(height_mm-ysize);
  XuViewport(xoff, yoff, xsize, ysize);
  XuViewWorldLimits(xmin, xmax, ymax, ymin, 0, 0);
  XuClip(XuON);
  XuViewWorkbox (1., 1., 0.); 
  XuViewWorld();
  
  double lw, linewidth;
  parameter->get("linewidth", &linewidth);
  // char text[10];
  Pointer<EdgeClass> ed;
  Pointer<VertexClass> v;

  int freqmax = 0;
  for (ed=net->edges; ed; ++ed) {
    if (ed->frequency>freqmax) freqmax = ed->frequency;
  }
    
  int npoints, i;  
  for (ed=net->edges; ed; ++ed) { // kanten malen
    lw = 3.0 * linewidth * double(ed->frequency)/double(freqmax) + 0.5;
    printf("f=%d lw=%lf\n", ed->frequency, lw);
    XuWidthColor(lw, 1); 
    npoints = ed->vertices.N();
    float *xarr = new float[npoints];
    float *yarr = new float[npoints];
    i = 0;
    for (v=ed->vertices; v; ++v) {
      xarr[i] = float(v->P.x);
      yarr[i] = float(v->P.y);
      ++i;
    }
    XuLineDraw (xarr, yarr, npoints);
    delete xarr;
    delete yarr;
  }
}

NetPlot::~NetPlot() {
  XuClose();
}


