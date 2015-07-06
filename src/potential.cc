#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <agX/agx.h>
#include "vector.hh"

Vector X(0.0,0.0), V(1.0,0.0), E0(1.0,0.0);
double delta = 1.0;
double rmax = 10.0;
double sigma = 1.0;
double v = 1.0;

Vector F(Vector Xb) {
  Vector D = Xb - X;
  double d1 = D.length();
  //Vector VT = V * delta; // alte version Ellipse mit momentaner Geschw.
  Vector VT = E0 * v * delta; 
  Vector D2 = D-VT;
  double d2 = D2.length(); // d2 = mass(dx-vtx,dy-vty);
  double a = d1+d2, r = sqrt(a*a - VT*VT)/2.0;
  double gamma = exp(-r/sigma) / sigma * a/r;       
  return (D/d1 + D2/d2) * gamma; // fx = gamma*(dx/d1 + (dx-vtx)/d2);
}     

int main (int argc, char** argv) {

  float width_mm, height_mm;
  int width_pix, height_pix;
  char *comm = new char[32];
  
  int NX = 40, NY = 40;
  double xmin=-5.0, xmax=5.0, ymin=-5.0, ymax=5.0;
  
  float vector[2*NY*NX];
#define vec(n,j,i) vector[(n*NY+j)*NX+i]
  float xscale, yscale, yexag, maxdx;
  float xsize, ysize, xsi, ysi;
  float xoff, yoff;

  printf("Eingabe: v, sigma, dt\n");
  scanf("%lf,%lf,%lf", &v, &sigma, &delta);

  XuInitialize(argc, argv);
  XuDeviceSelect(comm);
  XuOpen(XuCREATE);
  XuColorDefaultLoad(32);
  XuDrawingAreaQuery(&width_mm, &height_mm, &width_pix, &height_pix);

  if ((ymax-ymin)<(xmax-xmin)) {
    /* X ist laenger */
    xsize = width_mm>height_mm ? width_mm : height_mm;
    xsize *= 0.9;
    ysize = (ymax-ymin)/(xmax-xmin)*xsize;
  }
  else {
    /* Y ist laenger */
    ysize =width_mm>height_mm ? width_mm : height_mm;
    ysize *= 0.9;
    xsize = (xmax-xmin)/(ymax-ymin)*ysize;
  }

  xoff = 0.5*(width_mm-xsize);
  yoff = 0.5*(height_mm-ysize);

  XuViewWorldLimits(xmin, xmax, ymax, ymin, 0, 0);
  XuViewport(xoff, yoff, xsize, ysize);
  XuClip(XuON);
  /* XuViewWorkbox (1., 1., 0.); */ 

  double xx, yy;
  Vector f;
  double dx = (xmax - xmin)/(NX-1);
  double dy = (ymax -ymin)/(NY-1);
  maxdx=0.0;
  for (int j=0; j<NY; j++) {
    yy = ymin + dy * j;
    for (int i=0; i<NX; i++) {
      xx = xmin + dx * i;
      f = F(Vector(xx, yy));
      vec(0,j,i) = f.x;
      vec(1,j,i) = f.y;
      if (fabs(f.x)>maxdx) maxdx = fabs(f.x);
    }
  }
  
   yexag = ysize/xsize * (xmax-xmin)/(ymax-ymin);

/* Draw a frame around viewport */
   XuMapBorderFrameWidth(0.5);

/* Select Blocked data layout */
   XuSymbolDataLayoutSelect(XuSYMBOLBLOCKED);

/* Select symbol to Cartesian Arrow with solid head */
   XuSymbolSelect(XuSYMBOLARROW, -2);

/* Set the X scaling of the arrows so that the maximum delta X
   value corresponds to 12% of the viewport. For the Y component
   we have to take the Y vieport size and the Y exageration into
   account. */
   xscale = maxdx/0.12;
   yscale = xscale * ysize/xsize / yexag;

   XuSymbolAttrPlotScale(1, XuSYMBOLXSCALE, xscale);
   XuSymbolAttrPlotScale(2, XuSYMBOLYSCALE, yscale);

  /* Draw the arrows */
  XuSymbolRegDraw(vector, 2, NY, NX);

  XuViewWorld();
  XuCircleDraw(X.x, X.y, -0.35, 1, -1.0);
  Vector VT = X + E0 * v * delta;
  XuCircleDraw(VT.x, VT.y, -0.35, 0, -1.0);
  
  XuClose();
}

