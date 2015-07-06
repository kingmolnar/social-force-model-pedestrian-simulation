#define NTEST
#include "test.h"
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>  
#include <sys/stat.h>  
//#include <unistd.h>  

//#include "attraction.hh"
#include "floor.hh"
#include "street.hh"
#include "ground.hh"
#include "pedestrian.hh"

SimulObject* SimulCreatorStreet::create(SimulObjectType y) {
  SimulObject* obj = NULL;
  switch (y) {
  case _UNDEF:
  default: 
    printf("unknown object-type.\n");
    obj = NULL;
    break;
    //  case _ENVIRONMENT:
    //obj = new Environment(parobj);
    //break;
  case _STREET:
    obj = new Street;
    break;
  case _CORRIDOR:
    obj = new Corridor;
    break;
  case _POLYGON:
    obj = new Polygon;
    break;
  case _CIRCLE:
    obj = new Circle;
    break;
  case _DECORATION:
    obj = new Decoration;
    break;
  case _GATE:
    obj = new Gate;
    break;
  case _MULTIPLEGATE:
    obj = new MultipleGate;
    break;
  case _DOOR:
    obj = new Door;
    break;
  } // end switch
  return obj;
}

SimulObjectType SimulCreatorStreet::type(char *token) {
  SimulObjectType t = _UNDEF;
  if (strcasecmp(token, "street")==0) t = _STREET;
  else if (strcasecmp(token, "corridor")==0) t = _CORRIDOR;
  else if (strcasecmp(token, "polygon")==0) t = _POLYGON;
  else if (strcasecmp(token, "circle")==0) t = _CIRCLE;
  else if (strcasecmp(token, "decoration")==0) t = _DECORATION;
  else if (strcasecmp(token, "gate")==0) t = _GATE;
  else if (strcasecmp(token, "mgate")==0) t = _MULTIPLEGATE;
  else if (strcasecmp(token, "door")==0) t = _DOOR;
  return t;
}


bool intersect(Vector P, Vector Q, Vector X, Vector Y) {
  Vector Ep = Q - P;
  double a = Ep.length();
  Ep /= a;
  Vector Es = Vector(-Ep.y, Ep.x);
  X -= P; Y -=P;
  // notwendige Bedingung
  double xs = X*Es, ys = Y*Es, z=xs*xs;
  if (z*z<=0.0001) {
    // ein punkt ist auf der linie
    double xp = X*Ep, yp = Y*Ep;
    if ((xp>=0.0 && xp<=a) || (yp>=0.0 && yp<=a)) return 1;
    else return 0;
  }
  else if (z<=0.0001) {
    // vorzeichen wechsel
    Vector D = Y-X;
    double d = D.length();
    double l = (X+(D*xs/(ys-xs)))*Ep;
    // hinreichend
    if (l>=0 && l<=a) return 1;
    else return 0;
  }
  return 0;
}

bool intersect(Vector P, Vector Ep, double p, Vector Q, Vector Eq, double q) {
  Vector Es = Vector(-Ep.y, Ep.x);
  Vector X = Q-P;
  Vector Y = Q+(Eq*q)-P;
  // notwendige Bedingung
  double xs = X*Es, ys = Y*Es; 
  if (xs*ys<=0.0) {
    double l = (X+(Eq*q*xs/(ys-xs)))*Ep;
    // hinreichend
    if (l>=0 && l<=p) return 1;
  }
  return 0;
}
  
// OBSTACLE 
Vector Obstacle::F(Vector X) {
  Vector D = dest(X);
  double r = D.length();
  return (r>0.0 && r<rmax) ?
    -D/r * beta * exp(-r/sigma)/sigma : Vector(0.0,0.0);
}

double Obstacle::U(Vector X) {
  double r = dest(X).length();
  return r<0.0 ? ( r<rmax ? beta * exp(-r/sigma) : 0.0) : beta;
}

void Obstacle::print(FILE *fp) {
  fprintf(fp, "Obstacle\t%lf|%lf b=%lf s=%lf r=%lf c=%d\n",
	  P.x, P.y, beta, sigma, rmax, color);
}

// WALL
Polygon::Polygon() {
  //ParameterDef d[] = { {"beta", "1.0"},{"sig", "1.0"},{"color", "1"} };
  ParameterDef d[] = { {"polygon", "1"} };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));	
  else 
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));	
  //  objecttype = _POLYGON;
  street = NULL;
}

void Polygon::init() {
  SimulObject::init();
  announce("init Polygon\n");
  Wall* w;
  Vector P, Q, Ep;
  double a, epsilon, beta, sigma, rmax;
  int color;
  int i, nwall;
  if (parameter->nv<2) {
    return;
  }
  get(parameter, "beta", &beta);
  get(parameter, "sig", &sigma);
  get(parameter, "color", &color);
  get(parameter, "epsilon", &epsilon, 0.0001);
  int tmp_avoid; 
  get(parameter, "avoid", &tmp_avoid, 0);
  nwall = 1;
  int nw = parameter->nv;
  if (parameter->v[0] == parameter->v[nw-1]) nw--; //geschlossenes Poly
  if (nw>=2) {
    w = new Wall;
    w->N = nw-1;
    w->PP = new Vector[w->N];
    w->EEp = new Vector[w->N];
    w->aa = new double[w->N];
    for (int i=0; i<w->N; i++) {
      P = parameter->v[i]; 
      Q = parameter->v[i+1];
      Ep = Q - P;
      a = Ep.length();
      if (a>0.0) Ep /= a;
      w->PP[i] = P; w->EEp[i] = Ep; w->aa[i] = a;
    }
    rmax = -log(epsilon) * sigma;
    w->beta = beta; w->sigma = sigma; 
    w->rmax = rmax; w->color = color;
    w->avoid = tmp_avoid ? 1 : 0;
    street->obstacles.append(w);
  }
}

int Polygon::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (obj) {
    switch (type) {
    case _STREET:
      if (!street) {
	street = (Street*)obj;
	announce("polygon--->street\n");
      }
      else announce("polygon-/->street\n");
      good = 1;
      break;
    default:
      good = 0;
      break;
    }
  }
  return good ? 1 : SimulObject::ref(obj, type);
}
 

// WALL

/* Wall::Wall(Vector pp, Vector ep, double aa, 
	   double bb, double ss, double rr, int cc) {
  P = pp; Ep = ep; a = aa; beta = bb; sigma = ss; rmax = rr; color = cc;
  test(printf("create Wall %lf|%lf\n", P.x, P.y));
};
*/

Vector Wall::dest(Vector X) {
  Vector D, Dmin = dest(X,0);
  double d, dmin = Dmin.length();
  for (int i=1; i<N; ++i) {
    D = dest(X, i);
    d = D.length();
    if (d<dmin) {
      Dmin = D;
      dmin = d;
    }
  }
  return Dmin;
}


Vector Wall::dest(Vector X, int i) {
  Vector D;
  double cosa, r, d, l;
  D = X - PP[i];
  d  = D * EEp[i];
  if (d<0.0) {
    return -D;
  }
  else if (d<aa[i]) {
    r = D.length(); 
    cosa = d / r;
    l = sqrt(1.0-cosa*cosa) * r;
    Vector L;
    L.x = -l*EEp[i].y;
    L.y = l*EEp[i].x;
    double p = D*L;
    return L * (p > 0.0 ? -1.0 : 1.0);  
  }
  else {
    return PP[i] + (EEp[i]*aa[i]) - X;
  }
}

int Wall::intersect(Obstacle* obs) {
  return 0;
}

int Wall::intersect(Vector X, Vector Y) {
  for (int i=0; i<N; ++i) {
    if (intersect(X, Y, i)) return 1;
  }
  return 0;
}
  

int Wall::intersect(Vector X, Vector Y, int i) {
  Vector D = Y-X;
  double d = D.length();
  Vector Es = Vector(-EEp[i].y, EEp[i].x);
  double q = D*Es;
  if (q==0.0) return 0;
  double lambda = (PP[i]*Es + X*Es)/q;
  if (lambda>=0 && lambda<=D.length()) return 1;
  return 0;
}

void Wall::print(FILE *fp) {
  Vector  Q = PP[0] + EEp[0] *aa[0];
  fprintf(fp, "polygon {\n");
  for (int i=0; i<N; ++i) {
    Q = PP[i] + EEp[i]*aa[i];
    fprintf(fp, "%lf|%lf; ",  PP[i].x, PP[i].y, Q.x, Q.y);
  }
  fprintf(fp, " b=%lf s=%lf r=%lf c=%d\n",
	  P.x, P.y, Q.x, Q.y, beta, sigma, rmax, color);
}

// CIRCLE

Circle::Circle() {
  ParameterDef d[] = { {"circle", "1"} };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));	
  else 
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));	
  //  objecttype = _CIRCLE;
  street = NULL;
}

void Circle::init() {
  SimulObject::init();
  announce("init circle\n");
  double epsilon;
  CWall *cw;
  
  cw = new CWall;
  if (parameter->nv>0) cw->P = parameter->v[0];
  if (parameter->na>0) cw->radius = parameter->a[0];

  get(parameter, "beta", &(cw->beta));
  get(parameter, "sig", &(cw->sigma));
  get(parameter, "color", &(cw->color));
  get(parameter, "epsilon", &epsilon);
  cw->rmax = -log(epsilon) * cw->sigma;
  int tmp_avoid; 
  get(parameter, "avoid", &tmp_avoid, 0);
  cw->avoid = tmp_avoid ? 1 : 0;
  street->obstacles.append(cw);
}

int Circle::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (obj) {
    switch (type) {
    case _STREET:
      if (!street) {
	street = (Street*)obj;
	announce("circle--->street\n");
      }
      else announce("circle-/->street\n");
      good = 1;
      break;
    default:
      good = 0;
      break;
    }
  }
  return good ? 1 : SimulObject::ref(obj, type);
}

//CWall::CWall(Vector _p, double _a, double _b, double _s, double _r, int _c) {
//P = _p; radius = _a; beta = _b; sigma = _s; rmax = _r; color = _c;
//};

Vector CWall::dest(Vector X) {
  Vector D;
  double r, d;
  D = P - X;
  d = D.length();
  if (d>0.0) {
    r = d - radius; // Abst zum Mittelpkt - Radius 
    D *= r/d;
    return D;
  }
  else return Vector(0.0,0.0);
}

// Vector  Circle::F(Vector) {} double  Circle::U(Vector) {}

int CWall::intersect(Obstacle*) {
  return 0;
}

int CWall::intersect(Vector X, Vector Y) {
  Vector Ep = Y - X;
  double ep = Ep.length();
  if (ep>0.0) {
    Ep /= ep;
    Vector Es = !Ep;
    if (fabs((P-X)*Es) > radius) return 0;
    double l = (P-X)*Ep;
    if (l<0.0 || l>ep) return 0;
    return 1;
  }
  return 0;
}

void CWall::print(FILE *fp) {
  fprintf(fp, "CWall\t%lf|%lf, %lf b=%lf s=%lf r=%lf c=%d\n",
	  P.x, P.y, radius, beta, sigma, rmax, color);
}

//DECORATION
Decoration::Decoration() {
  ParameterDef d[] = { {"deco", "1"} };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));	
  else 
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));	
//  objecttype = _DECORATION;
}

void Decoration::init() {
  SimulObject::init();
  N = parameter->nv;
  if (!N) {
    type = DecoNULL;
    return;
  }
  x = new float[N];
  y = new float[N];
  for (int i=0; i<N; ++i) {
    x[i] = float(parameter->v[i].x);
    y[i] = float(parameter->v[i].y);
  }
  if (N>=2) {
    type = DecoPOLYGON;
  }
  else if (parameter->na>=1) {
    type = DecoCIRCLE;
    radius = parameter->a[0];
  }
  else {
    type = DecoTEXT;
  }
  get(parameter, "color", &color, 1);
  get(parameter, "framecolor", &framecolor, 1);
  get(parameter, "framewidth", &framewidth, 0.15);
  get(parameter, "texthight", &textheight, 1.0);
}

  
//Street
Street::Street() {
  //   ParameterDef d[] = { {"xmin", "./."},
  // 		       {"xmax", "./."},
  // 		       {"dx", "0.5|0.5"},
  // 		       {"beta", "./."},
  // 		       {"sig", "./."},
  // 		       {"eps", "./."},
  // 		       {"force", "min"},
  // 		       {"color", "1"},
  // 		     };
  if (!parameter)
    parameter = new Parameter(); 
  // d, sizeof(d)/sizeof(ParameterDef));
  //   else
  //     parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  //  objecttype = _STREET;
  floor = NULL;
  u = NULL; 
}

Street::~Street() {
  if (u) delete u;
}
  
void Street::init() {
  SimulObject::init();
  announce("init street\n");
  parameter->get("xmin", &Xmin);
  parameter->get("xmax", &Xmax);
  parameter->get("dx", &Dx);
  parameter->get("force", &forcefct);
  test(printf("number of obstacles = %d\n", obstacles.N()));
  // clear data
  E.clear(); V.clear(); S.clear();
  // while(pedestrians()) pedestrians.remove();
}


int Street::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (obj) {
    switch (type) {
    case _CIRCLE:
    case _POLYGON:
      announce("street--->obstacle\n");
      heap.append(obj);
      good = 1;
      break;
    case _DECORATION:
      decorations.append(obj);
      good = 1;
      break;
    case _GATE:
      gates.append(obj);
      good = 1;
      break;
    case _ATTRACTION:
      attractions.append(obj);
      good = 1;
      break;
    case _PEDESTRIAN:
      /* announce("street--->pedestrian\n");
      pedestrians.append(obj);
      good =1; */
      break;
    case _STREET:
      adjacents.append(obj);
      good = 1;
      break;
    case _FLOOR:
      floor = (Floor*)obj;
      good = 1;
      break;
    case _ENTRY:
    case _DESTINATION:
    default:
      good = 0;
      break;
    }
  }
  return good ? 1 : SimulObject::ref(obj, type);
}


//void Street::calc() {
//}

/* {
  int ix, iy, Nx, Ny;
  double x, y;
  FILE *fp;
  char fname[32];
  
  if (s) {
    Nx = (s->xmax - s->xmin)/s->dx + 1;
    Ny = (s->ymax - s->ymin)/s->dy + 1;
    if (s->u) free(s->u);
    s->u = (float*)flooroc(Nx*Ny*sizeof(float));
    sprintf(fname, "xsim.street.%ld", s->id); 
    fp = fopen(fname, "w");
    fprintf(fp, "# nx=%d ny=%d\n", Nx, Ny); 
    for (ix=0; ix<Nx; ix++) {
      for (iy=0; iy<Ny; iy++) {
	x = s->xmin+ix*s->dx;
	y = s->ymin+iy*s->dy;
	uu(ix,iy) = Ubuild(s, x, y);
	fprintf(fp,"%lf %lf %lf\n", x, y, uu(ix,iy));
      }
      fprintf(fp,"\n"); 
    }
    fclose(fp);
    printf("end calc\n");
    /// plot(s);  
  }
  }
  */
//
//   Potential- und Kraefte-Funktionen
//

double Street::U(Vector X) {
  Pointer<Obstacle> o;
  double utemp, ures = 0.0, rtemp, r = rmax;
  switch (forcefct) {
  case 0: // maximales Hindernis
    for (o=obstacles; o; ++o) {
      utemp = o->U(X);
      if (utemp > ures) ures = utemp;
    }
    break;
  case 1: // Summe 
    for (o=obstacles; o; ++o) {
      ures += o->U(X);
    }
    break;
  case 2: // minimaler Abstand
    for (o=obstacles; o; ++o) {
      rtemp = o->dest(X).length();
      if (rtemp < r) r = rtemp;
    }
    if (r<rmax && r>0.0) ures = beta * exp(-r/sigma)/sigma;
    break;
  }
  return ures;
}

Vector Street::F(Vector X) {
  Pointer<Obstacle> o;
  Vector Ftemp, Fres(0.0, 0.0);
  double r = rmax;
  switch (forcefct) {
  case 0: // maximales Hindernis
    for (o=obstacles; o; ++o) {
      Ftemp = o->F(X);
      if (Ftemp.length() > Fres.length()) Fres = Ftemp;
    }
    break;
  case 1: // Summe 
    for (o=obstacles; o; ++o) {
      Fres += o->F(X);
    }
    break;
  case 2: // minimaler Abstand
    for (o=obstacles; o; ++o) {
      Ftemp = o->dest(X);
      if (Ftemp.length() < r) {
	Fres = Ftemp;
	r = Ftemp.length();
      }
    }
    if (r<rmax && r>0.0) Fres = Fres/r * beta * exp(-r/sigma)/sigma;
    else Fres = Vector(0.0,0.0);
    break;
  }
  return Fres;
}

Vector Street::F(Vector X, Vector Ex, double ws) {
  Pointer<Obstacle> o;
  Vector Ftemp, Fres(0.0, 0.0);
  double r = rmax;
  switch (forcefct) {
  case 0: // maximales Hindernis
    for (o=obstacles; o; ++o) {
      Ftemp = o->F(X);
      if ((Ftemp*Ex) > 0.0) Ftemp *= ws;
      if (Ftemp.length() > Fres.length()) Fres = Ftemp;
    }
    break;
  case 1: // Summe 
    for (o=obstacles; o; ++o) {
      Ftemp = o->F(X);
      if ((Ftemp*Ex) > 0.0) Ftemp *= ws;
      Fres += Ftemp;
    }
    break;
  case 2: // minimaler Abstand
    for (o=obstacles; o; ++o) {
      Ftemp = o->dest(X);
      if (Ftemp.length() < r && Ftemp*Ex < 0.0) {
	Fres = Ftemp;
	r = Ftemp.length();
      }
    }
    if (r<rmax && r>0.0) Fres = Fres/r * beta * exp(-r/sigma)/sigma;
    else Fres = Vector(0.0,0.0);
    break;
  }
  return Fres;
}

void Street::print(FILE *fp) {
  fprintf(fp, "Street\t(%lf|%lf, %lf|%lf)", Xmin.x, Xmin.y, Xmax.x, Xmax.y);
  for (Pointer<Obstacle> o = obstacles; o; ++o) {
    o->print(fp);
  }
}

// Corridor
Corridor::Corridor() {
  grid = NULL;
  part_rho = NULL;
  faktor = NULL;
  X = NULL;
  val = NULL;
  Nparts = 0;
  performance = NULL;
  stream = NULL;
  ParameterDef d[] = { {"dx", "0.5|0.5"}
		     };
  if (!parameter) 
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else 
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  //  objecttype = _CORRIDOR;
}
  
Corridor::~Corridor() {
  if (grid) delete grid;
  if (part_rho) delete part_rho;
  if (faktor) delete faktor;
  if (performance) {
    for (int i=0; i<Nparts; ++i) delete performance[i];
    delete performance;
  }
  if (stream) fclose(stream);
}


void Corridor::init() {
  Street::init();
  char *filename;
  get(parameter, "filename", &filename, "");
  if (!stream && *filename) stream = fopen(filename, "w"); 
  int i;
  if (Nparts==0) { // die Felder werden neu angelegt 
    get(parameter, "nparts", &Nparts, 1);
    grid = new QGrid(Xmin, Xmax, Dx);
    int N = grid->N();
    int NN = N*Nparts;
    part_rho = new double[NN];
    for (i=0; i<NN; ++i) part_rho[i] = 0.0;
    faktor = new double[N];
    for (i=0; i<N; ++i) faktor[i] = 1.0; // hier nachher noch die Begrenzungen
    performance = new DataPointer[Nparts];
    for (i=0; i<Nparts; ++i) performance[i] = new Data(Nvariables);
    neg_entropy = 0.0;
  }
  else { // nur loeschen
    int NN = grid->N()*Nparts;
    for (i=0; i<NN; ++i) part_rho[i] = 0.0; // Dichte
    for (i=0; i<Nparts; ++i) performance[i]->clear();
  }

  // Markierung
  if (grid) {
    get(parameter, "mbeta", &mbeta);
    get(parameter, "msigma", &msigma);
    double epsilon;
    get(parameter, "epsilon", &epsilon, 0.001);
    double rmax = -log(epsilon) * msigma; // Reichweite des  Potentials
    int nx = int(rmax / Dx.x);
    int Nx = 2*nx+1;
    int ny = int(rmax / Dx.y);
    int Ny = 2*ny+1;
    number = Nx*Ny;
    val = new double[number];
    X = new Vector[number];
    int x, y, j = 0;
    for (x=-nx; x<=nx; ++x) {
      for (y=-ny; y<=ny; ++y) {
	X[i].x = Dx.x * x;
	X[i].y = Dx.y * y;
	val[i] = mbeta*exp(-X[i].length()/msigma);
	++i;
      }
    } 
  }
}

void Corridor::data(int cl, Vector V /* position */, 
		    double v, double v0, double f, double r) { 
  if (cl<0 || cl>=Nparts) cl = 1;
  // Statistik
  if (performance) {
    double x[Nvariables];
    x[0] = v;
    x[1] = v0;
    x[2] = f;
    x[3] = r;
    performance[cl]->data(x);
  }
  // Dichte
  int pos;
  if (grid && X && val && grid->inside(V, &pos)) {
    // und das sollte immer der Fall sein
    double c = faktor[pos];
    int N = grid->N();
    for (int k=0; k<number; k++)
      if (grid->inside(V+X[k], &pos)) part_rho[cl*N+pos] += val[k] * c; 
  }
}

void Corridor::prologue() {
  // Vorbereitung vor Zeitschritt
  // alle Daten werden auf Null gesetzt
  int i;
  if (performance) for (i=0; i<Nparts; ++i) performance[i]->clear();
  if (grid && part_rho) {
    int NN = grid->N() * Nparts;
    for (i=0; i<NN; ++i) part_rho[i] = 0.0;
  }
}

void Corridor::epilogue() {
  // Nach dem Zeitschritt
  // Berechnung der Entropie, Index ueber alle Clans und alle Rasterknoten
  int i;
  if (grid && part_rho) {
    double r;
    double P = 0.0;
    double PP = 0.0;
    int NN = grid->N() * Nparts;
    for (i=0; i<NN; ++i) {
      r = part_rho[i];
      if (r > 1.0e-15) {
	P += r;
	PP += r*log(r);
      }
    }
    if (P > 1.0e-15) neg_entropy = log(P) + PP/P;
    else neg_entropy = 0.0;
  }
  // Statistik: muss eigentlich nichts gemacht werden.
}

void Corridor::report(FILE *s) {
  if (!s) s = stream;
  if (s) {
    for (int cl=0; cl<Nparts; ++cl) {
      if (performance[cl]->N()>0) {
	fprintf(s, "%3d:", cl);
	fprintf(s, " %15.6lf %15.6lf", floor->T, neg_entropy);
	fprintf(s, " %6d", performance[cl]->N());
	for (int i=0; i<Nvariables; ++i) 
	  fprintf(s, " %15.6lf %15.6lf", performance[cl]->mean(i),
		  performance[cl]->var(i));
	fprintf(s, "\n");
      }
    }
  }
  else {
    printf("%lf, %lf,", floor->T, neg_entropy);
    for (int cl=0; cl<Nparts; ++cl) {
      printf(" %d:", cl);
      //for (int i=0; i<Nvariables; ++i)
      int i = 0;
      printf(" %lf(%lf)", performance[cl]->mean(i),performance[cl]->var(i));
    }
    printf("\n");
  }
}

int LocationNummer = 1;
  
// Location

Location::Location() {
  street = new (Street*)[1];
  street[0] = NULL;
  nr = LocationNummer++;
  //  objecttype = _LOCATION;
}

void Location::init() {
  WithLogfile::init();
}

int Location::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;  
  switch (type) {
    //  case _ENVIRONMENT:
  case _STREET:
    street[0] = (Street*)obj;
    good = 1;
    break;
  default:
    break;
  }
  return good ? 1 : SimulObject::ref(obj, type);
}

bool Location::passed(Vector& X, Vector& Y) {
  return (dest(X)*dest(Y) <= 0.0);
}

void Location::print(FILE *fp) {
  fprintf(fp, "Location\t%c=%d\n", color);
}

// Gate
Gate::Gate() {
  street = new (Street*)[2];
  floor = NULL;
  street[0] = street[1] = NULL;
  if (!parameter) 
    parameter = new Parameter();
}

void Gate::init() {
  Location::init();
  vel.clear();
  test(printf("init Gate %d", parameter->nv));
  switch (parameter->nv) {
  case 2:
    test(printf("%lf|%lf, %lf%lf", 
		parameter->v[0].x, parameter->v[0].y,  
		parameter->v[1].x, parameter->v[1].y));
    P = parameter->v[0];
    Q = parameter->v[1];
    break;
  case 1:
    P = Q = parameter->v[0];
    break;
  default:
    parameter->get("p", &P);
    parameter->get("q", &Q);
    break;
  }
  parameter->get("funct", &funct);
  parameter->get("color", &color);
  double tau, dt;
  //get(parameter, "tau", &tau);get(parameter, "dt", &dt);dec = exp(-dt/tau);
  get(parameter, "radius", &radius, 0.0);
  get(parameter, "vmax", &vmax, (floor?floor->vmax:1.0));
  announce("\n");
}

int Gate::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;  
  switch (type) {
    //  case _ENVIRONMENT:
  case _STREET:
  { 
    Street *str = (Street*)obj;
    if (!street[0]) {
      street[0] = str;
      good = 1;
    }
    else if (!street[1]) { 
      street[1] = str;
      good = 1;
    }
    else good = 0;
  }
    break;
  case _FLOOR: 
    floor = (Floor*)obj;
    good = 1;
    break;
  default:
    break;
  }
  return good ? 1 : Location::ref(obj, type);
}

int Gate::simul(Pedestrian *p)
{
  /*   Floor *floor = p->floor;
       int i;
       double Trel, Drel;
       i = int((p->v0 - floor->vmin)/(floor->vmax - floor->vmin) * numberv);
       vcount[i]++;
       Trel = p->Tneeded/p->Texpected;
       Drel = p->Dneeded/(p->Texpected*p->v0);
       TrelMean[i] = (TrelMean[i] * (vcount[i]-1) + Trel) / vcount[i];
       DrelMean[i] = (DrelMean[i] * (vcount[i]-1) + Drel) / vcount[i];
       
       TrelM = TrelV = DrelM = DrelV = 0.0;
       for (i=0; i<numberv; i++ ) {
       TrelM += TrelMean[i];
       DrelM += DrelMean[i];
       }
       TrelM /= numberv;
       DrelM /= numberv; */
  return(0);
}

Vector Gate::rndpos() {
  Vector Ep = Q - P;
  double h = rnduniform(0.0, 1.0);
  return P + Ep*h;
}

Vector Gate::dest(Vector& X) {
  Vector D = X - P;
  Vector Ep = Q - P;
  double a = Ep.length();
  if (a<=0.0) return D;
  Ep /= a;
  double d  = D * Ep;
  if (d<0.0) {
    return -D;
  }
  else if (d<a) {
    double r = D.length(); 
    double cosa = d / r;
    double l = sqrt(1.0-cosa*cosa) * r;
    return !Ep * l * (D*!Ep > 0.0 ? -1.0 : 1.0); 
    // Vector(l*Ep.y, -l*Ep.x); // senkrecht
  }
  else {
    return P + (Ep*a) - X;
  }
}

void Gate::velocity(Vector& D, Vector& V) {
  if (D.length() <= radius) vel.data(V);
}

/* Vector Gate::dest(Gate *gat) {
  Vector R0 = dest(gat->P);
  Vector R1 = dest(gat->Q);
  if (R0.length() < R1.length()) return R0;
  else return R1;
} */ 

/* bool Gate::passed(Walker *w) {
  if (dest(W)*E0 <= 0.0) {
    // durchschritten
    Vel += V;
    return 1;
  }
  return 0;
} */

/* bool Gate::passed(Walker *w) {// intersect
  Vector Y = w->X, X = w->Xtail[w->itail];
  Vector Ep = Q - P;
  double a = Ep.length();
  Ep /= a;
  Vector Es = Vector(-Ep.y, Ep.x);
  double y = (Y-P)*Es;
  double x = (X-P)*Es;
  if (x*y==0.0) return 1;
  if (x*y>0.0) return 0; // auf der gleichen Seite
  Vector D = Y-X;
  double d = D.length();
  double lamda = (X+D*(x/y)-P)*Ep;
  if (lamda>=0 && lamda<=d) return 1;
  return 0;
} */ 

bool Gate::passed(Vector& X, Vector& Y) {// intersect
  return intersect(P, Q, X, Y);
}

void Gate::log() {
  if (logfp) fprintf(logfp, "%lf\t%lf\t%lf\n", (floor?floor->T:0.0),
		     vel.mean().x, vel.mean().y);
}

void Gate::print(FILE *fp) {
  fprintf(fp, "Gate\t%lf|%lf, %lf|%lf c=%d\n",
	  P.x, P.y, Q.x, Q.y, color);
}
 
// Alternative Multiple Gate

MultipleGate::MultipleGate() {
}

MultipleGate::~MultipleGate() {
}

void MultipleGate::init() {
  Gate::init();
  get(parameter, "xsi", &xsi, 1.0);
  get(parameter, "lambda", &lambda, 1.0);
  // kommt noch was ?
}

int MultipleGate::ref(SimulObject *obj, SimulObjectType t) {
  int good = 0;  
  if (obj) {
    switch (t) {
    case _GATE:
      gates.append(obj);
      good = 1;
      break;
    default:
      good = 0;
      break;
    }
  }
  return good ? 1 : Gate::ref(obj, t);
}


Vector MultipleGate::dest(Vector& X) {
  Pointer<Gate> gat = gates;
  Vector D, Dmin = gat->dest(X);
  for (++gat; gat; ++gat) {
    D = gat->dest(X);
    if (D.length()<Dmin.length()) Dmin = D;
  }
  return Dmin;
}


Gate& MultipleGate::operator[](int i) {
  Pointer<Gate> gat = gates;
  while (i--) ++gat;
  return *gat;
}

/* Vector MultipleGate::dest(Walker *w) {
  const double destmin = 0.02;
  int n = gates.N();
  if (!n) return Vector(0.0,0.0);
  double p[n]; // double *n = new double[n];
  Vector D[n];
  Pointer<Gate> gat;
  int i = 0;
  double d[n], t[n], tmax, tsumme = 0.0;
  Vector Ed;
  for (gat=gates; gat; ++gat, ++i) {
    D[i] = gat->dest(w);
    d[i] = D[i].length();
    Ed = d[i]>0.0 ? D[i]/d[i] : Vector(0.0,0.0); 
    //t[i] = gat->vel.N()? (gat->vel.mean()*D[i])/(d[i]*d[i]): w->v0/d[i];
    // 4. t = (v0-v)/v0 >= 0.0 fuer v<=v0
    t[i] = gat->vel.N() ? (w->v0 - gat->vel.mean()*Ed) / w->v0 : 0.0;
    if (t[i]<0.0) t[i] = 0.0; 
    tmax = (i==0)? t[0]:((t[i]>tmax)? t[i]:tmax);
    tsumme += t[i];
  }
  // double tempmax = 100.0*xsi;  double temp;
  for (i=0; i<n; i++) {
    // 1. Gauss 
    // p[i] = exp(-t[i]*t[i]/(2*xsi*xsi));
    // 2. exponenatial
    //p[i] = exp(t[i]/xsi);
    // 3. exponential & skaliert
    //p[i] = exp((t[i]-tmax)/tsumme/xsi);
    // 4. 
    p[i] = exp(-d[i]*(1.0+t[i])/xsi);
    printf("%d %lf %lf, ", i, t[i], p[i]);
  }
  int sel = irndsel(p,n);
  printf("-> %d\n", sel);
  return D[sel];
} */

/* Vector MultipleGate::dest(Walker *w) {
  const double destmin = 0.02;
  int n = gates.N();
  if (!n) return Vector(0.0,0.0);
  double p[n]; // double *n = new double[n];
  Vector D[n];
  Pointer<Gate> gat;
  int i = 0;
  double d[n], tt;
  Vector S, Es;
  double s, dmin;
  for (gat=gates; gat; ++gat, ++i) {
    S = gat->Q - gat->P; S = !S; s = S.length(); 
    D[i] = gat->dest(w);
    d[i] = D[i].length();
    Es = D[i]*S < 0.0 ? -S/s : S/s;
    tt = gat->vel.N() ? fabs(w->v0- gat->vel.mean()*Es) / w->v0 : 0.0;
    if (tt<0.0) tt = 0.0;
    d[i] *= (1.0+tt*lambda);
    dmin = i==0 ? d[i] : (d[i]<dmin ? d[i] : dmin); 
    //p[i] = exp(-d[i]/xsi); // absoluter Abstand
  }
  for (i=0; i<n; i++) {
    p[i] = exp(-(d[i]-dmin)/xsi); // Differenz Abstand
    //printf("%d %lf %lf, ", i, d[i], p[i]);
  }
  int sel = irndsel(p,n);
  //printf("%d\n", sel);
  return D[sel];
  } */

// neuer Versuch mit WarteZeit
/* Vector MultipleGate::dest(Walker *w) {
  const double destmin = 0.02;
  int n = gates.N();
  if (!n) return Vector(0.0,0.0);
  double p[n]; // double *n = new double[n];
  Vector D[n];
  Pointer<Gate> gat;
  int i = 0;
  double t[n], tt;
  Vector S, Es;
  double s, tmin;
  for (gat=gates; gat; ++gat, ++i) {
    S = gat->Q - gat->P; S = !S; s = S.length(); 
    D[i] = gat->dest(w);
    t[i] = D[i].length(); ///w->v0;
    Es = D[i]*S < 0.0 ? -S/s : S/s;
    tt = gat->vel.N() ? (w->v0 - gat->vel.mean()*Es) / w->v0 : 0.0;
    if (tt<0.0) tt = 0.0;
    t[i] += tt*lambda; 
    p[i] = exp(-t[i]/xsi);
    //tmin = i==0 ? t[i] : (t[i]<tmin ? t[i] : tmin); 
  }
  //for (i=0; i<n; i++) { p[i] = exp(-(t[i]-tmin)/xsi); // Differenz Abstand
  //printf("%d %lf %lf,", i, t[i], p[i]); }
  int sel = irndsel(p,n);
  //printf("%d\n", sel);
  return D[sel];
} */


bool MultipleGate::passed(Vector& X, Vector& Y) {
  for (Pointer<Gate> gat=gates; gat; ++gat) {
    if (gat->passed(X, Y)) return 1;
  }
  return 0;
}


Door::Door() {
}

Door::~Door() {
}

void Door::init() {
  Gate::init();
  Vector D = Q-P;
  double d = D.length();
  double d2 = d/2.0;
  double offset;
  get(parameter, "offset", &offset, d2);
  if (offset<=0.0) {
    flag.zero = 1;
    flag.punkt = 0;
  }
  else if (offset>=d2 || d==0.0) { // ein punkt
    flag.zero = 0;
    flag.punkt = 1;
    Qp = Pp = P+D/2.0;
    roffset = d2;
  }
  else {
    flag.zero = 0;
    flag.punkt = 0;
    Qp = P+D*(1-offset/d);
    Pp = P+D*(offset/d);
    roffset = d2-offset;
  }
}

Vector Door::rndpos() {
  Vector Ep = Qp - Pp;
  double h = rnduniform(0.0, 1.0);
  return P + Ep*h;
}

Vector Door::dest(Vector& X) {
  if (flag.zero) return Gate::dest(X);
  Vector R;
  if (flag.punkt) {
    R = Pp-X; //    goto weiter;
  }
  else {
    R = X - Pp;
    Vector Ep = Qp - Pp;
    double a = Ep.length();
    if (a>0.0) {// goto weiter; //return D;
      Ep /= a;
      double d  = R * Ep;
      if (d<0.0) {
	R = -R; // goto weiter; // return -D;
      }
      else if (d<a) {
	double r = R.length(); 
	double cosa = d / r;
	double l = sqrt(1.0-cosa*cosa) * r;
	R = !Ep * l * (R*!Ep > 0.0 ? -1.0 : 1.0); //	goto weiter;
	// Vector(l*Ep.y, -l*Ep.x); // senkrecht
      }
      else {
	R = Pp + (Ep*a) - X;
      }
    }
  }
  if (R.length()<roffset) return Gate::dest(X);
  else return R;
}

/* end of file */










