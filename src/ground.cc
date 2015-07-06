/* Ground class */
#include "test.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

/* #include "pedestrian.h" */
#include "simul.hh"
#include "ground.hh"
#include "floor.hh"

SimulObject* SimulCreatorGround::create(SimulObjectType y) {
  SimulObject* obj = NULL;
  switch (y) {
  case _UNDEF:
  default: 
    printf("unknown object-type.\n");
    obj = NULL;
    break;
  case _GROUND:
    obj = new Ground;
    break;
  case _MARKER:
    obj = new Marker;
    break;
  } // end switch
  return obj;
}

SimulObjectType SimulCreatorGround::type(char *token) {
  SimulObjectType t = _UNDEF;
  if (strcasecmp(token, "ground")==0) t = _GROUND;
  else if (strcasecmp(token, "marker")==0) t = _MARKER;
  return t;
}

Marker::Marker() {
  ground = NULL;
  floor = NULL;
  val = NULL;
  ParameterDef d[] = { {"beta", "1.0"},
		       {"sigma", "2.0"}
		     };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  // objecttype = _MARKER;
}

Marker::~Marker() {
  if (val) delete val;
  number = 0;
}

void Marker::init() {
  SimulObject::init();
  int fl = parameter->flags;
  get(parameter, "flag", &intflag, fl);
  get(parameter, "beta", &beta);
  get(parameter, "sigma", &sigma);
  double epsilon;
  get(parameter, "epsilon", &epsilon, 0.001);
  double rmax = -log(epsilon) * sigma; // Reichweite des  Potentials
  int nx = int(rmax / ground->Dx.x);
  int Nx = 2*nx+1;
  int ny = int(rmax / ground->Dx.y);
  int Ny = 2*ny+1;
  number = Nx*Ny;
  // Values
  val = new double[number];
  // Offsets
  I = new IntVector[number];
  X = new Vector[number];
  int ix, iy, i = 0;
  for (ix=-nx; ix<=nx; ++ix) {
    for (iy=-ny; iy<=ny; ++iy) {
      I[i].x = ix; I[i].y = iy;
      X[i].x = ground->Dx.x * ix;
      X[i].y = ground->Dx.y * iy;
      val[i] = beta*exp(-X[i].length()/sigma);
      //test(printf("%d, %d|%d %lf|%lf %lf\n",i,I[i].x,I[i].y,X[i].x,X[i].y,val[i]));
      ++i;
    }
  } 
  double dt;
  get(parameter, "dt", &dt, floor ? floor->Dt : 0.1);
  double delay;
  get(parameter, "delay", &delay, 0.0);
  nring = int(delay/dt);
  printf("delay=%lf, n=%d\n", delay, nring);
  if (nring>0) {
    array = new struct RingElement[nring];
    get(parameter, "nmax", &Nmax, 100);
    for (int i=0; i<nring; ++i) {
      array[i].X = new Vector[Nmax];
      array[i].a = new double[Nmax];
      array[i].N = 0;
    }
    iring = 0;
  }
}

int Marker::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (!obj) return good;
  switch (type) { 
  case _GROUND:
    ground = (Ground*)obj;
    good = 1;
    break;
  case _FLOOR:
    floor = (Floor*)obj;
    good = 1;
    break;
  default:
    good = 0;
    break;
  }
  return good ? 1 : SimulObject::ref(obj, type);
}

void Marker::add(Vector V) {
  int j;
  for (int k=0; k<number; k++) {
    if (ground->grid->inside(V+X[k], &j)) ground->g[j] += val[k];
  }
}

void Marker::add(Vector V, double m) {
  int j;
  for (int k=0; k<number; k++) {
    if (ground->grid->inside(V+X[k], &j)) ground->g[j] += val[k] * m;
  }
}

void Marker::stamp(Vector X, double a) {
  if (array[iring].N<Nmax) {
    array[iring].X[array[iring].N] = X;
    array[iring].a[array[iring].N] = exp(a/sigma);
    array[iring].N++;
  }
}

void Marker::stamp(Vector X, Vector V, double v0) {
  double m;
  switch (flag.faktor) {
  case 0:
    m = exp((V.length()-v0)*floor->Dt/sigma);
    break;
  case 1:
    m = V.length()/v0;
    break;
  }
  if (array[iring].N<Nmax) {
    array[iring].X[array[iring].N] = X;
    array[iring].a[array[iring].N] = m;
    array[iring].N++;
  }
}

void Marker::sub(Vector X) {
  int pos = ground->grid->indx(X);
  int j, ng = ground->grid->N();
  for (int k=0; k<number; k++) {
    j = pos+ground->grid->indx(I[k]);
    if (j>=0 && j<ng) ground->g[j] -= val[k];
  }
}

void Marker::sub(Vector X, double m) {
  int pos = ground->grid->indx(X);
  int j, ng = ground->grid->N();
  for (int k=0; k<number; k++) {
    j = pos+ground->grid->indx(I[k]);
    if (j>=0 && j<ng) ground->g[j] -= val[k] * m;
  }
}

void Marker::update() {
  if (nring==0) return;
  iring = (iring+1)%nring;
  int j, nn = array[iring].N;
  for (j=0; j<nn; j++) {
    add(array[iring].X[j], array[iring].a[j]);
  }
  array[iring].N = 0;
}

QGrid::QGrid(Vector& A, Vector& B, Vector& C) {
  Xmin = A;
  Xmax = B;
  Dx = C;
  N.x = int((Xmax.x-Xmin.x)/Dx.x)+1;
  N.y = int((Xmax.y-Xmin.y)/Dx.y)+1;
  Npoints = N.x * N.y;
  test(printf("construct grid nx=%d, ny=%d\n", N.x, N.y));
}

int QGrid::indx(Vector Pos) {
  Vector D = Pos - Xmin;
  IntVector I;
  I.x = int(D.x/Dx.x);
  I.y = int(D.y/Dx.y);
  //if (I.x >=0 && I.x < N.x && I.y >=0 && I.y < N.y) 
  return indx(I);
  //else return -1;
}

int QGrid::inside(IntVector Ipos, int *x) {
  *x = indx(Ipos);
  return (Ipos.x>=0 && Ipos.x<N.x && Ipos.y>=0 && Ipos.y<N.y);
}

int QGrid::inside(int ix, int iy, int *x) {
  *x = indx(ix, iy);
  return (ix>=0 && ix<N.x && iy>=0 && iy<N.y);
}

int QGrid::inside(Vector Pos, int *x) {
  Vector D = Pos - Xmin;
  IntVector I;
  I.x = int(D.x/Dx.x);
  I.y = int(D.y/Dx.y);
  *x = indx(I);
  return (I.x >=0 && I.x < N.x && I.y >=0 && I.y < N.y);
}
    
Ground::Ground() {
  floor = NULL;
  grid = NULL;
  g = NULL;
  ParameterDef d[] = { {"decay", "1.0"}
		     };
  if (!parameter)
    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  else
    parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  //  objecttype = _GROUND;
}

Ground::~Ground() {
  if (grid) delete grid;
  if (g) delete g;
}

void Ground::decay() {
  if (!floor) return;
  if (T < floor->T && dec>=0.0) {
    if ((dec * gmax == gmax) && (dec * gmin == gmin)) {
      /* MessageLine("decay skiped"); */
    }
    else {
      int i, n = grid->N();
      for (i=0; i<n; i++) g[i] *= dec;
      if (flag.autoscale) {
	gmin *= dec; gmax *= dec;
      }
    }
    T = floor->T + timeSkip * floor->Dt;
  }
}

void Ground::adjust() {
  if (!flag.autoscale) return;
  int i, n = grid->N();
  for (i=0; i<n; i++) {
    if (g[i] < gmin) gmin = g[i];
    if (g[i] > gmax) gmax = g[i];
  } 
}

double Ground::U(Vector& X) {
  int pos;
  if (g && grid->inside(X, &pos)) return g[pos];
  else return 0.0;
}

Vector Ground::F(Vector& V) {
  int pos;
  if (g && grid->inside(V, &pos)) { 
    double z = g[pos];
    double xm = grid->inside(V+Vector(-Dx.x,0.0), &pos) ? g[pos] : z;
    double xp = grid->inside(V+Vector( Dx.x,0.0), &pos) ? g[pos] : z;
    double ym = grid->inside(V+Vector(0.0,-Dx.y), &pos) ? g[pos] : z;
    double yp = grid->inside(V+Vector(0.0, Dx.y), &pos) ? g[pos] : z;
    return Vector(-0.5*(xp-xm)/Dx.x, -0.5*(yp-ym)/Dx.y);
  }
  else return Vector(0.0,0.0);
}

void Ground::init() {
  SimulObject::init();
  int fl = parameter->flags;
  get(parameter, "flag", &intflag, fl);
  get(parameter, "xmin", &Xmin);
  get(parameter, "xmax", &Xmax);
  get(parameter, "dx", &Dx);
  assert(grid==NULL);
  grid = new QGrid(Xmin, Xmax, Dx);
  g = new double[grid->N()];
  get(parameter, "min", &gmin, 0.0);
  get(parameter, "max", &gmax, 0.0);
  get(parameter, "decayrate", &timeSkip, 1);
  get(parameter, "tau", &tau, 1.0);
  double dt;
  get(parameter, "dt", &dt, floor ? floor->Dt : 0.1);
  dec = exp(-dt*timeSkip/tau);
  // "dec"
  get(parameter, "beta", &beta, 0.0);
  test(printf("init ground npoints=%d dec=%lf\n", grid->N(), dec));
}

int Ground::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (!obj) return good;
  switch (type) { 
  case _FLOOR:
    floor = (Floor*)obj;
    good = 1;
    break;
  default:
    good = 0;
    break;
  }
  return good ? 1 : SimulObject::ref(obj, type);
}

int Ground::mwrite(int fd) {
  int rc=0;
  register int i;
  register double scale;
  unsigned char ch;

  if (fd==-1) {
    return 2*4 + 2*4 + 2*4 + 4 + 4 + grid->N();
  }
  else {
    rc += write_netbor4(fd,(double*) &Xmin, 2)<<2;
    rc += write_netbor4(fd,(double*) &Xmax, 2)<<2;
    rc += write_netbor4(fd,(double*) &Dx, 2)<<2;

    rc += write_netbor4(fd,gmin)<<2;
    rc += write_netbor4(fd,gmax)<<2;

    scale=(gmax-gmin)/256;
    for (i=0; i<grid->N(); ++i) {
      ch=(unsigned char)((g[i]-gmin)/scale);
      rc += ::write(fd,&ch,1);
    }
  }
}

int Ground::mread(int fd, int num) {
  int rc=0;
  register int i;
  register double scale;
  unsigned char ch;

  rc += read_netbor4(fd,(double*) &Xmin, 2)<<2;
  rc += read_netbor4(fd,(double*) &Xmax, 2)<<2;
  rc += read_netbor4(fd,(double*) &Dx, 2)<<2;

  rc += read_netbor4(fd,&gmin)<<2;
  rc += read_netbor4(fd,&gmax)<<2;

  scale=(gmax-gmin)/256;
  for (i=0; i<grid->N(); ++i) {
    rc += ::read(fd,&ch,1);
    g[i]=(double)ch*scale+gmin;
  }
}
