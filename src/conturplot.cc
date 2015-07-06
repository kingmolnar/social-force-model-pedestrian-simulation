// header
class ConturPlot: public Plot {
  Floor *floor;
  List streets;
  Ground *ground;
  float xmin, xmax, ymin, ymax, zmin, zmax, *z, zz;
  struct {
    unsigned int color : 1;
    unsigned int range : 1;
  } flag;  
public:
  ConturPlot();
  void init();
  int ref(SimulObject*);
  void plot();
}; 

class GateStatPlot: public Plot {
public:
  GateStatPlot(Parameter*, SimulObject*);
  void plot();
}; 







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
  if (parameter->b) delete parameter->b;
  parameter->b = new bool[16];
  objecttype = _CONTUR_PLOT;
}

void ConturPlot::init() {
  parameter->get("command", &comm);
  flag.color = parameter->b[1];
}

int ConturPlot::ref(SimulObject *obj) {
  int good = 0;
  switch (obj->objecttype) {
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
 
  if (ground) { 
#ifdef _GROUND_HH_
  /* case _STREET:
     street = (Street*)simobj;
     if (!parameter->b[4]) {
     xmin = street->Xmin.x;
     xmax = street->Xmax.x;
     ymin = street->Xmin.y;
     ymax = street->Xmax.y;
     dx = street->Dx.x;
     dy = street->Dx.y;
     }
     // berechne z
     nx = (xmax - xmin)/dx + 1;
     ny = (ymax - ymin)/dy + 1;
     
     z = new float[nx*ny];
     zmin = zmax = street->U(Vector(xmin,ymin)); 
     for (ix=0; ix<nx; ix++) {
     for (iy=0; iy<ny; iy++) {
     x = xmin+ix*dx;
     y = ymin+iy*dy;
     zz = z[ix+iy*nx] = street->U(Vector(x,y));
     if (zz<zmin) zmin = zz;
     if (zz>zmax) zmax = zz;
     }
     }
     break;
     #ifdef _GROUND_HH_
     case _GROUND:
     break;
     #endif 
     */  
  
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
  
  xoff = 0.5*(plot->width_mm-xsize);
  yoff = 0.5*(plot->height_mm-ysize);
  
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

GateStatPlot::GateStatPlot(Parameter *p, SimulObject *obj) {
  parameter = p;
  simobj = obj;
}

GateStatPlot::plot() {
}; 
