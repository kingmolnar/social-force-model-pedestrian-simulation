/* Pedestrian class */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream.h>
 
#define NTEST
#include "test.h"
#include <assert.h>

#include "pedestrian.hh"
#include "floor.hh"
#include "street.hh"
#include "ground.hh"

//extern  double drnnof_();
//extern  void rnset_(int*), rnopt_(int*);

extern "C" void bcopy(char *b1, char *b2, int length);

#define sq(X)  ((X)*(X))
#define mass2(X,Y) ((X)*(X)+(Y)*(Y))
#define mass(X,Y)  sqrt(mass2(X,Y))
#define UNEND 1.0e20


#define zufall(A,B) ((B - A)*rand()/2147483647.0 + A)

#ifndef XuF_BREAK
#define XuF_BREAK 999.999
#endif

// Creator-Klasse
// ist fuer alle Objektklassen dieser Programmdatei zustaendig

SimulObject* SimulCreatorPedestrian::create(SimulObjectType y) {
  SimulObject* obj = NULL;
  switch (y) {
  case _UNDEF:
  default: 
    printf("unknown object-type.\n");
    obj = NULL;
    break;
  case _PEDESTRIAN:
    obj = new Pedestrian;
    break;
  case _EVALPEDEST:
    obj = new EvalPed;
    break;
    /* case _DATAPEDEST:
       obj = new DataPedest;
       break; */ 
  } // end switch
  return obj;
}

SimulObjectType SimulCreatorPedestrian::type(char *token) {
  SimulObjectType t = _UNDEF;
  if (strcasecmp(token, "pedestrian")==0) t = _PEDESTRIAN;
  else if (strcasecmp(token, "evalpedest")==0) t = _EVALPEDEST;
  else if (strcasecmp(token, "pedestrianpop")==0) t = _PED_POP;
  else if (strcasecmp(token, "datapedest")==0) t = _DATAPEDEST;
  return t;
}


int WalkerNummer = 1;

Walker::Walker() {
  Xtail = NULL;
  X = X0 = Xp = V = E0 = NULL;
  v0 = NULL;
  itail = NULL;
  Ntail = NULL;
  flag = NULL;
  /* ParameterDef *d = NULL;
     if (!parameter)
     parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
     else
     parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
     */
  nr = WalkerNummer++;
}


Walker::~Walker() {
}

void Walker::init() {
  Actor::init();
  Ntail = 0;
  double tail, dt;
  get(parameter, "dt", &dt, 0.1);
  get(parameter, "tail", &tail, 0.0);
  if (dt>0.0 && tail>0.0) Ntail = int(tail/dt);
  if (Ntail<=0) Ntail = 1;
  
  if (popsize()) {
    X  =    new Vector[popsize()];
    X0 =    new Vector[popsize()];
    Xp =    new Vector[popsize()];
    V =     new Vector[popsize()];
    E0 =    new Vector[popsize()];
    v0 =    new double[popsize()];
    flag =  new WalkerFlag[popsize()];
    Xtail = new (Vector*)[popsize()];
    itail = new int[popsize()];
    for (int j=0; j<popsize(); ++j) {
      Xtail[j] = new Vector[Ntail];
      itail[j] = 0;
    }
  }
}

void Walker::clock() {
  Actor::clock();
  for (int w=0; w<posize(); ++w) {
    X[w].update();
    X0[w].update();
    Xp[w].update();
    V[w].update();
    E0[w].update();
  }
}
  
int Walker::mwrite(int fd) {
  int rc=0;

  // write out to fd this walker, if fd=-1 don't write but return
  // how much we'd write if fd was != -1.

  // first write Data of superceding type
  rc += Actor::mwrite(fd);

  // now walker data, output always twice as much doubles than popsize()
  // reports as each vector consists of two coordinates.
  if (fd!=-1) {
    rc += write_netbor4(fd,(double*)X, popsize()*2)<<2;
    rc += write_netbor4(fd,(double*)V, popsize()*2)<<2;
    rc += write_netbor4(fd,(double*)v0, popsize())<<2;
    for(int i=0; i<popsize(); ++i) {
      unsigned char c='\000';

      c|=(flag->visible)?1:0;
      c|=(flag->active)?2:0;
      rc+= ::write(fd,&c,1);
    }
    rc += write_netbor2(fd,&clan, popsize())<<1;
  } 	
  else {
    rc += 2+21*popsize();
  }
  return rc;
}

int Walker::mread(int fd, int num) {
  int rc=0,tmp,i;

  //read the superceding type, save current popsize.
  tmp=popsize();

  // cerr<<"Reading "<<num<<" bytes for pedestrian."<<endl;
  rc += Actor::mread(fd,num);
  if (rc==-1) return rc;

  // check if the pop size has grown, if yes: delete data structure and
  // create a new one with the larger size.
  // Ignore if it has shrunk. This is a waste of memory but a gain in time.
  if (popsize()>tmp) {
    //   cerr<<"Warning: popsize grew! allocating more memory."<<endl;
    //   cerr<<"Old size: "<<tmp<<" New size: "<<popsize()<<endl;
    //   cerr<<"Current position in input stream"<<::tell(fd)<<'.'<<endl;

    for (i=0;i<tmp;++i) delete [] Xtail[i];
    delete [] Xtail;
    delete [] X;
    delete [] X0;
    delete [] Xp;
    delete [] V;
    delete [] E0;
    delete [] v0;
    delete [] flag;
    delete [] itail;
    X     = new Vector[popsize()];
    X0    = new Vector[popsize()];
    Xp    = new Vector[popsize()];
    V     = new Vector[popsize()];
    E0    = new Vector[popsize()];
    v0    = new double[popsize()];
    flag  = new WalkerFlag[popsize()];
    itail = new int[popsize()];
    Xtail = new (Vector*)[popsize()];
    for (i=0;i<popsize();++i) {
      Xtail [i] = new Vector[Ntail];
      itail [i] = 0;
    }
  }

  // now read the data of a walker:
  // if (num-rc<(3*4*2+1)*popsize()) return rc; // there are not enough data

  tmp=rc;
  rc+=read_netbor4(fd,(double*)X,popsize()*2)<<2;
  rc+=read_netbor4(fd,(double*)V,popsize()*2)<<2;
  rc+=read_netbor4(fd,v0,popsize())<<2;
  for(i=0; i<popsize(); ++i) {
    unsigned char c;
    rc+= ::read(fd,&c,1);

    flag->visible=(c & (unsigned char)1)?1:0;
    flag->active=(c & (unsigned char)2)?1:0;
  }
  rc += read_netbor2(fd,&clan,(int)popsize())<<1;
  if ( ! color ) color=clan;
  tmp+=1+21*popsize();

  // rc should be equal to tmp, otherwise this indicates an error.
  
// DON'T DO THAT ANY MORE
// If pending bytes are left they MUST be read by the caller.
// If we stripped them here we possibly removed data of a superceding
// type.
//  if(num-tmp) {
//    // read pending bytes:
//    char* buf;
//    buf = new char [num-tmp];
//    rc += ::read(fd,buf,num-tmp);
//    delete [] buf;
//    // cerr<<"Adjusting "<<num-tmp<<" pending bytes."<<endl;
//  }

  return rc;
}

int Walker::write(int stream) {
  int w, n=0;
  for (w=0; w<popsize(); ++w) {
    if (flag[w].visible) { 
      /*  WalkerRecordBlock wrb;
	  wrb.type = WRBdata;
	  wrb.continued = 0;
	  wrb.data.clan = clan;
	  wrb.data.x = X.x;
	  wrb.data.y = X.y;
	  wrb.data.vx = V.x;
	  wrb.data.vy = V.y;
	  wrb.data.v0 = v0;
	  wrb.data.from_gate = 0;
	  wrb.data.to_gate = 0;
	  return wrb.write(stream); */
    }
  }
  return n;
}

/* int Walker::read(int stream) {
  WalkerRecordBlock wrb;
  wrb.continued = 1;
  int sum = 0;
  while (wrb.continued) {
    int res = wrb.read(stream);
    if (res<=0) return sum;
    sum += res;
    switch (wrb.type) {
    default:
    case WRBmisc:
      break;
    case WRBdata:
      clan = wrb.data.clan;
      X.x = wrb.data.x;
      X.y = wrb.data.y;
      V.x = wrb.data.vx; 
      V.y = wrb.data.vx;
      v0 = wrb.data.v0;
      break;
    case WRBdata2:
      break;
    case WRBdata3:
      break;
    }
  }
  return sum;
} */



Pedestrian::Pedestrian() {
  floor = NULL;
  street = NULL;
  next = NULL;
  vmax = delay = T = NULL;
  Tneeded = Dneeded = Texpected = Ttotal = Ttotex = Dtotal = NULL;
  dest = orig = NULL;
  eta = zeta = zeta_ped = zeta_build = zeta_attr = zeta_group 
    = negentropy = xidata = NULL;
//  Y = NULL;
//  P = Pac = NULL;
 Nclans = 1;
}
 
Pedestrian::Pedestrian(Pedestrian* ped) {
  parameter = ped->parameter;
  parent = ped->parent;
  // feste Listen kopieren
//  grounds = ped->grounds;
//  markers = ped->markers;
//  friends = ped->friends;
//  if (friends()) {
//    friends.append(this);
//  }
  // nicht feste Listen kopieren
#ifdef _ATTRACTION_HH_
  if (ped->attractions()) {
    for (Pointer<Attraction> pa = ped->attractions; pa; ++pa) {
      attractions.append(pa);
    }
  }
#endif
}


Pedestrian::~Pedestrian() {
}

void Pedestrian::init() {
  Walker::init();
  get(parameter, "groupsize", &grpsize, 1);
  if (grpsize<1) grpsize = 1;
  int p = (popsize()/grpsize)*grpsize;
  popsize(p);
  if (popsize()) {
    vmax =      new double[popsize()];
    tau =       new double[popsize()];
    delay =     new double[popsize()];
    T =         new double[popsize()];
    Tneeded =   new double[popsize()];
    Dneeded =   new double[popsize()];
    Texpected = new double[popsize()];
    Ttotal =    new double[popsize()];
    Ttotex =    new double[popsize()];
    Dtotal =    new double[popsize()];
    dest = new Pointer<Gate>[popsize()];
    orig = new Pointer<Gate>[popsize()];
    eta =  new Data[popsize()];
    zeta =  new Data[popsize()];
    zeta_ped   = new Data[popsize()];
    zeta_build = new Data[popsize()];
    zeta_attr  = new Data[popsize()];
    zeta_group = new Data[popsize()];
    negentropy = new Data[popsize()];
    xidata     = new Data[popsize()];
    int i;
    NY = 4;
    Y.dim(NY, popsize());

    get(parameter, "nclans", &Nclans, 1);
    if (Nclans<1) Nclans = 1;
    P.dim(Nclans, popsize());
    Pac.dim(Nclans, popsize()); 
  }
  Pedestrian::reset();
}

void Pedestrian::reset(int w) {
  //Attraction *att;
  //double abstand, mx, my, t, r0;
  int i, tmp;
  static double _v, _vmin, _vf;
  static WalkerFlag _flag;
  //  int *pflag = (int*)&_flag;
  if (w<0) {
    get(parameter, "vmin", &_vmin, 0.0);
    get(parameter, "vfaktor", &_vf, 1.0);
    get(parameter, "pi", &pi, 1.0);
    get(parameter, "mu", &mu, 0.0);
    get(parameter, "lam", &lam, 0.0);
    get(parameter, "gam", &gam, 0.0);
    get(parameter, "delta", &delta, 0.0);
    get(parameter, "weightshift", &weightShift, 1.0);
    get(parameter, "width", &width, 1.0);
    get(parameter, "sig", &sigma, 1.0);
    double epsilon;
    get(parameter, "epsilon", &epsilon, 0.0001);
    rmax = -log(epsilon) * sigma; // Reichw. d. Fussgaenger-Pot.
    
    get(parameter, "color", &color, 1);
    // get(parameter, "fct", pflag, 0);
    // if (*pflag) cerr<<"Warning: fct is obsolete and CPU dependent."<<endl;
    //tmp=_flag.visible?1:0;
    get(parameter,"visible",&tmp, 1);
    _flag.visible = tmp ? 1 : 0;

    get(parameter,"xray",&tmp, 0);
    _flag.xray = tmp ? 1 : 0;

    get(parameter,"periodic",&tmp, 0);
    _flag.periodic = tmp ? 1 : 0;;

    get(parameter,"active",&tmp, 1);	// default for active is always 1
    _flag.active = tmp ? 1 : 0;

    get(parameter,"ellip",&tmp, 0);	// default for active is always 0
    _flag.ellip = tmp ? 1 : 0;

    get(parameter,"sum",&tmp, 0);	// default for active is always 0
    _flag.sum = tmp ? 1 : 0;

    //  parameter, "draw", &draw_flag);
    get(parameter, "clan", &clan, 0);
    
    int ww;
    for (ww=0; ww<popsize(); ww += grpsize) Pedestrian::reset(ww);

    if (street) {
      street->pedestrians.append(this);
    }
    
    objecttask = SET_VALUES;
  }
  else { // w>=0
    bool isfirst = grpsize>1 ? (w%grpsize == 0) : 1; // ist der erste!
    int w1 = (w/grpsize)*grpsize;
    int w2 = w1+grpsize;
    int wi;
    
    get(parameter, "vmax", &vmax[w], 10.0e20);
    get(parameter, "v0", &v0[w], 0.0);
    _v = v0[w];
    while (v0[w]<_vmin || v0[w]>vmax[w]) {
      get(parameter, "v0", &v0[w], 0.0);
      if (v0[w]==_v) break;
      _v = v0[w];
    }
    vmax[w] = v0[w] * _vf;
    flag[w] = _flag;
    double vtau;
    get(parameter, "vtau", &vtau, 2.0);
    get(parameter, "tau", &tau[w], vtau>0.0? v0[w]/vtau: 0.5);
    
    if (isfirst) {
      get(parameter, "t", &T[w], 0.0); 
      get(parameter, "delay", &delay[w], 0.0);
    }
    else {
      for (wi=w1+1; wi<w2; ++wi) {
	T[wi] = T[w1];
	delay[wi] = delay[w1];
      }
    }
    // loop
    //    for (wi=w1; wi<w2; ++wi) { 
    //Flags 
    flag[w].visible = (T[w]<=0.0); 
    flag[w].reset = 1;
    
    int n_gates = gates.N();
    if (n_gates==0) {
      parameter->get("x", &X[w]);
      parameter->get("x0", &X0[w]);
    }
    else if (n_gates==1) {
      parameter->get("x", &X[w]);
      dest[w] = gates;
    }
    else if (n_gates>=2) {
      orig[w] = gates;
      dest[w] = gates; ++dest[w];
      X[w] = orig[w]->rndpos();
    }
    V[w] = Vector(0.0, 0.0);
    for (i=0; i<Ntail; ++i) Xtail[w][i] = X[w]; 
    itail[w] = 0;
    
    // Attractions
    /* ped->alpha = (double*)flooroc(street->Natts*sizeof(double));
       for (att=street->att; att; att=att->succ) 
       ped->alpha[att->index] = att->alpha0; */
    // restliche Parameter
    
    if (isfirst) for (wi=w1+1; wi<w2; ++wi) reset(wi);
  } // if w>=0
}


int Pedestrian::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  if (!obj) return good;
  switch (type) { 
//  case _ATTRACTION:
//    attractions.append(obj);
//    good = 1;
//    break;
  case _GATE:
    gates.append(obj);
    good = 1;
    break; 
#ifdef _GROUND_HH_
  case _GROUND:
    grounds.append(obj);
    good = 1;
    break;
  case _MARKER:
    markers.append(obj);
    good = 1;
    break;
#endif
//  case _PEDESTRIAN:
//    friends.append(obj);
//    good = 1;
//    break;
#ifdef _ROUTE_HH_
  case _NETWORK:
    if (!network) {
      network = (Network*)obj;
      good = 1;
    }
    else good = 0;
    break;
#endif
  case _FLOOR:
    floor = (Floor*)obj;
    good = 1;
    break;
  case _STREET:
    if (!street) {
      street = (Street*)obj;
      good = 1;
    }
    else good = 0;
    break;
  case _POLYGON:
  case _CIRCLE:
  case _OBSTACLE:
  case _UNDEF:
  default:
    good = 0;
    break;
  }
  if (good) return 1;
  else return Walker::ref(obj, type);
}


// Berechne die Kraftfunktion zwischen zwei Fussgaengern
// neues Potential

int Pedestrian::update() {
  double fx, fy, _fp, Fx, Fy, v, weight, dx0, dy0, r0, ex0, ey0;
  Vector FF, f, Xd, dV, Es;
  double F2, S;
  int nF;
  double m, m_rezi, l;
  int i, j, k, n;
  Pointer<Pedestrian> p;

  int w;
  
  switch (objecttask) {
  case SET_VALUES:
  case RESET:
    // Anpassen der Parameter, die von anderen Objekten abhaengen
    // if  (vmax > floor->vmax) vmax = floor->vmax;  
    for (w=0; w<popsize(); ++w) flag[w].reset = 1;
    /* Zeichnen */
    //  draw_flag |= CuDontDrawTrace;
    
    objecttask = RUN;
    // no break!
  case RUN:
    //extern FILE *Logfp;
    n = 0;
    for (w=0; w<popsize(); ++w) {
      // falls Fussgaenger noch im Rennen 
      if (flag[w].reset) {
	if (dest[w]) Texpected[w] = dest[w]->dest(X[w]).length()/v0[w];
	else Texpected[w] = r0;
	Tneeded[w] = 0.0;
	Dneeded[w] = 0.0;
	Ttotal[w] = 0.0;
	Ttotex[w] = 0.0;
	Dtotal[w] = 0.0;
	flag[w].reset = 0;
	eta[w].clear();
	zeta[w].clear();
	zeta_ped[w].clear();
	zeta_build[w].clear();
	zeta_attr[w].clear();
	zeta_group[w].clear();
 	negentropy[w].clear();
	xidata[w].clear();
	for (int i=0; i<NY; ++i) Y(i,w).clear();
	for (int j=0; j<Nclans; ++j) P(j,w) = Pac(j,w) = 0.0; 
      }
      if (flag[w].active) { // (flag & PedActive) {      
	if (T[w] >= floor->T) {
	  //Fussgaenger in Warteposition
	  n++;
	}
	else {
	  flag[w].visible = 1;
	  //      draw_flag &= ~CuDontDrawTrace;
	  // Entfernung zum Ziel 
	  //abstand = floor->route.distance(orig, dest);
	  //Xd = X0 - X;
	  if (dest[w]) {
	    //cout << "ped " << w << " Nalt " << dest[w]->Nalt() << endl;
	    Xd = dest[w]->dest(X[w]);
	    dest[w]->velocity(Xd, V[w]);
	    X0[w] = X[w] + Xd;
	  }
	  else {
	    Xd = X0[w] - X[w];
	  }
	  r0 = Xd.length();
	  // if (r0<=0.0) /* goto PED_ABB*/;//evtl. Abbruch, wenn Ziel erreicht
	  E0[w] = r0>0.01? Xd/r0: E0[w]; // Vector(0.0,0.0);
	  Es = Vector(-E0[w].y, E0[w].x); Es /= Es.length();
	  v = V[w].length();     
	  
	  // START
	  FF = Vector(0.0,0.0);
	  F2 = 0.0;
	  nF = 0;
	  // Ground-Potential
	  for (Pointer<Ground> gr=grounds; gr; ++gr) {
	    FF += f = gr->F(X[w]);
	    F2 += f.x*f.x+f.y*f.y;
	    ++nF;
	  }
	  // Hindernis
	  if (flag[w].sum) {
	    for (Pointer<Obstacle> o=street->obstacles; o; ++o) {
	      Vector D = o->dest(X[w]);
	      double gamma; 
	      if (o->avoid) {
		D.x = D * Es; D.y = D * E0[w];
		double gamma = 1.0 + V[w].length() * lam;
		if (D.y > 0.0) D.y /= (gamma*gamma);
	      }
	      double r = D.length();
	      if (r>0.0 && r<o->rmax) {
		f = -D * (o->beta/o->sigma/r * exp(-r/o->sigma));
		//if ((f*E0) > 0.0) f*= weightShift;
		if (o->avoid) {
		  f = Es*f.x + E0[w]*f.y; // beta * ?? 
		}
		FF += f;
		F2 += f.x*f.x+f.y*f.y;
		++nF;
	      }
	    }
	  }
	  else { // minimum abstand
	    Pointer<Obstacle> o=street->obstacles;
	    Obstacle *obs = o();
	    Vector D1, D;
	    if (obs) D = obs->dest(X[w]);
	    double r1, r = D.length();
	    while (++o) {
	      D1 = o->dest(X[w]);
	      r1 = D1.length();
	      if (r1<r) {
		r = r1;
		D = D1;
		obs = o();
	      }
	    }
	    if (obs) {
	      if (obs->avoid) {
		D.x = D * Es; D.y = D * E0[w];
		double gamma = 1.0 + V[w].length() * lam;
		if (D.y > 0.0) D.y /= (gamma*gamma);
	      }
	      r = D.length();
	      if (r>0.0 && r<obs->rmax) {
		f = -D * (obs->beta/obs->sigma/r * exp(-r/obs->sigma));
		if (obs->avoid) {
		  f = Es*f.x + E0[w]*f.y; // beta * ?? 
		}
		FF += f;
		F2 += f.x*f.x+f.y*f.y;
		++nF;
	      }
	    }
 	  }
	  
	  // Fussgaenger-Abstossung 
	  //	  double P[Nclans]; // proportion
	  FF += f = Fped(w, &F2, &nF);

	  // Varianz
	  if (nF) {
	    // zeta[w].data(F2/double(nF)
	    //   -(FF.x*FF.x+FF.y+FF.y)/double(nF*nF));
	    Y(1,w).data(F2/double(nF)-(f.x*f.x+f.y+f.y)/double(nF*nF));
	  }
	  // entropy
	  if (Nclans) {
	    // double y = 0.0;
	    for (i=0; i<Nclans; ++i) {
	      // y += P[i]>0.0 ? P[i] * log(P[i]) : 0.0;
	      Pac(i,w) += P(i,w) * floor->Dt; //Pac[i][w] += P[i] * floor->Dt;
	    }
	    //	    Y[3][w].data(y/double(Nclans));
	  }
	  
	  // Negentropy
	  // negentropy[w].data(S);

	  // Gruppenzusammenhalten
	  if (grpsize>1) {
	    double xi = 0.0;
	    int w1 = (w/grpsize)*grpsize; // erster der Gruppe
	    int w2 = w1+grpsize;
	    int i;
	    for (i=w1; i<w2; ++i) {
	      if (i==w) continue;
	      Vector R = X[w] - X[i];
	      double r = R.length();
	      xi += r;
	      if (r>0.0) {
		f = R * (-gam/r);
		FF += f;
		F2 += f.x*f.x+f.y*f.y;
		++nF;
	      }
	    }
	    // gesammelt wird fuer alle der Gruppe
	    for (i=w1; i<w2; ++i) Y(2,i).data(xi/double(grpsize-1));
	    // xidata[w].data(xi/double(grpsize-1));
	  }
	  // else { xidata[w].data(0.0); }
	  Y(2,w).data(0.0);
	  
	  // Fluktuation
	  if (mu>0.0) {
	    double f2 = FF*FF;
	    if (f2>0.0) {
	      double fluct = rndgauss(0.0, mu)*(FF*E0[w])/f2;
	      FF += Vector(fluct*-FF.y, fluct*FF.x);
	    }
	  }
	  
	  // Geschwindigkeitsaenderung
	  V[w] += ((E0[w]*v0[w] - V[w])/tau[w] + FF) * floor->Dt;
	  
	  // Theta-Funktion, Geschwindigkeit nach oben abschneiden 
	  v = V[w].length();     
	  if (v > vmax[w] && v > 0.0) {
	    V[w] = V[w] * vmax[w]/v;
	    v = vmax[w];
	  }
	  // eta[w].data( (E0[w]*V[w]) /v0[w] ); // Effizienz
	  // Effizienz & Varianz
	  Y(0,w).data((E0[w]*V[w]) / v0[w]);
	  // Orts-Aenderung
	  //Xp = X;
	  itail[w] = (itail[w]+1)%Ntail;
	  Xtail[w][itail[w]] = X[w];
	  X[w] += V[w] * floor->Dt;
	
	  // Markierungen
	  for (Pointer<Marker> mar=markers; mar; ++mar) {
	    //mar->stamp(X, (v-v0)*floor->Dt);
	    mar->stamp(X[w], V[w], v0[w]);
	  }
	
	  Dneeded[w] += V[w].length() * floor->Dt; //Dtotal  += v * floor->Dt;
	  // Timer 
	  T[w] = floor->T;
	  Tneeded[w] += floor->Dt;	//Ttotal  += floor->Dt;
	  
	  if (*street>=_CORRIDOR) {
	    ((Corridor*)street)->data(clan, X[w], v, v0[w], 0.0, 0.0);
	  }
	  
	  // Teste ob ein Tor durchschritten wurde, wenn ja: Neuer next.
	  
	  // Abbruchbedingung fuer Fussgaenger 
	  //if (dest && dest->dest(X).length() < V.length() * floor->Dt) {
	  if (dest[w] && dest[w]->passed(X[w], Xtail[w][itail[w]])) { 
	    // richtung hat sich geaendert.
	    // Tor erreicht, Zeitnehmer rausschreiben, naechstes Tor 
	    /* FORMAT NR T V0 Tex Tneed Dneed */
	    //	if (Logfp) fprintf(Logfp, "%d %lf %lf %lf %lf %lf \n", 
	    //	   dest->nr, T, v0, Texpected, Tneeded, Dneeded);
	    Ttotal[w] += Tneeded[w];
	    Ttotex[w] += Texpected[w];
	    Dtotal[w] += Dneeded[w];
	    //  printf("d=%lf t=%lf dtotex=%lf ttotex=%lf\n" ,Dtotal, Ttotal, Ttotex*v0, Ttotex); 
	    orig[w] = dest[w];
	    ++dest[w];
	    if (dest[w]) { // neues Ziel definieren
	      // neues Ziel, Statistik 
	      Texpected[w] = dest[w]->dest(X[w]).length()/v0[w];
	      Tneeded[w] = 0.0;
	      Dneeded[w] = 0.0;
	      n++;
	    } 
	    else {
	      // GesamtZeit
	      double PacSum = 0.0, YY[6];
	      YY[0] = Y(0,w).mean(); 
	      YY[1] = Y(0,w).var();
	      YY[2] = Y(1,w).mean();
	      YY[3] = Y(2,w).mean();
	      YY[4] = 0.0;
	      for (i=0;i<Nclans; ++i) PacSum += Pac(i,w); //Pac[i][w];
	      if (PacSum > 0.0) for (i=0;i<Nclans; ++i) {
		Pac(i,w) /= PacSum; //Pac[i][w] /= PacSum;
		YY[4] = Pac(i,w) > 0.0 ? Pac(i,w) * log(Pac(i,w)) : 0.0;
	      }
	      YY[4] = 1.0 + YY[4]/(double(Nclans)*Tneeded[w]);
	      YY[5] = Tneeded[w];
	      floor->journey(clan, v0[w], YY, 6);
// 	      floor->journey(clan, v0[w], eta[w].mean(), zeta[w].mean(), 
// 			     (Ttotal[w]>0 ?
// 			      negentropy[w].mean() - log(Ttotal[w]) : 0),
// 			     xidata[w].mean());
	      if (flag[w].periodic) { // kann wiedergeboren werden 
		//cout << "wiedergeburt\n"; 
		reset((w/grpsize)*grpsize); // immer den ersten
		n++; // weiter bei Reincarnation 
	      }
	      else {
		V[w] = Vector(0.0, 0.0);
		flag[w].active = 0; // flag &= ~PedActive;
	      }
	    }
	  }
	  else n++; // increment counter n, continue for n > 0 
	}
      } // if Active
    } // for w
    break;
  default:
    break;
  } // switch
  return(n);
}

// Fussgaenger Fkt.
double Pedestrian::U(Vector& Xb, int w) {
  double r, u = 0.0;
  if (w<0) for (int ww=0; ww<popsize(); ++ww) u+= U(Xb, ww);  
  else if (flag[w].visible) { // & PedVisible) {
    Vector D = Xb - X[w];
    Vector VT = V[w] * delta;
    r = sqrt(sq(D.length() + (D-VT).length()) - VT*VT)/2.0;
    u += -log(r)-log(sigma);
  }
  return u;
}

// Kraft von pedj auf ped 

Vector Pedestrian::F(Vector& Xb, int w) {
  /* Kraft auf Fg i */
  Vector f(0.0,0.0);
  if (w<0) for (int ww=0; ww<popsize(); ++ww) f+= F(Xb, ww);
  else if (flag[w].visible) {
    Vector D = Xb - X[w];
    double d1 = D.length();
    //Vector VT = V * delta; // alte version Ellipse mit momentaner Geschw.
    Vector VT = E0[w] * V[w].length() * delta; 
    Vector D2 = D-VT;
    double d2 = D2.length(); // d2 = mass(dx-vtx,dy-vty);
    if (d1<=rmax || d2<=rmax) {
      bool inters = 0;
      if (!flag[w].xray) {
	for (Pointer<Obstacle>obs = street->obstacles; obs; ++obs) {
	  if (inters |= obs->intersect(X[w], Xb)) break;
	}
      }
      if (!inters) {
	double a = d1+d2, r = sqrt(a*a - VT*VT)/2.0;
	double gamma = exp(-r/sigma) / sigma * a/r;       
	f += (D/d1 + D2/d2) * gamma; // fx = gamma*(dx/d1 + (dx-vtx)/d2);
      }
    }
  }
  return f;
}
// #define SPITZES_POTENTIAL
// #ifdef SPITZES_POTENTIAL
// Wechselwirkungskraefte 

Vector Pedestrian::Fped(int w, double *F2, int *nF) {
  Vector f(0.0, 0.0), f_hat, fprime;
  Vector R, R_hat, X_hat, Y_hat;
  double r, gamma, u, y_hat;
  bool inters;
  int v, c;
  //  double P[floor->Nclans];
  for (c=0; c<Nclans; ++c) P(c,w) = 0.0; // nP[c] = 0;}
  Pointer<Pedestrian> ped;
  Pointer<Obstacle> obs; 
  if (!flag[w].ellip) {
    for (ped=street->pedestrians; ped; ++ped) {
      for (v=0; v<ped->popsize(); ++v) {
	if ((ped()==this && w==v) || !ped->flag[v].visible) continue;
	R = X[w] - ped->X[v];
	r = R.length();
	if (r>rmax) continue;
	inters = 0;
	if (!flag[w].xray) {
	  for (obs = street->obstacles; obs; ++obs) {
	    if (inters |= obs->intersect(ped->X[v], X[w])) break;
	  }
	}
	if (inters) continue;
	// nichts hat sie aufgehalten
	// Y_hat = E0[w] * V[w].length() - ped->E0[v] * ped->V[v].length();
	// Y_hat = E0[w] -  ped->E0[v] * ped->V[v].length()/v0[w];
	// Y_hat = E0[w] -  ped->E0[v] * ped->v0[v]/v0[w];
	Y_hat = E0[w] -  (ped->E0[v] *(delta*ped->v0[v]/v0[w]));
	y_hat = Y_hat.length();
	if (y_hat>0.0) {
	  Y_hat /= y_hat;
	}
	else {
	  Y_hat = ped->X[v] - X[w];
	  y_hat = Y_hat.length();
	  if (y_hat>0.0) Y_hat /= y_hat;
	}
	X_hat = !Y_hat; // X_hat.x = - Y_hat.y; X_hat.y = Y_hat.x;
	X_hat /= X_hat.length();
	
	R_hat.x = X_hat * R; R_hat.y = Y_hat * R;
	// verkuerzung, verlaengerung
	gamma = 1.0 + V[w].length() * lam;
	if (R_hat.y < 0) R_hat.y /= (gamma*gamma);
	//else R_hat.y *= 1.0 + V[w].length() * lam;
	r = R_hat.length();
	u = pi * exp(-r/sigma); 
	P(ped->clan,w) += u;     // Entropie, Proportion
	gamma = 1 / sigma / r * u;
	f_hat =  R_hat * gamma; // X_hat * gamma + Y_hat * gamma; 
	f += fprime = X_hat * f_hat.x + Y_hat * f_hat.y;
	if (F2 && nF) {
	  *F2 += fprime.x * fprime.x + fprime.x * fprime.y;
	  ++(*nF);
	}
      }
    }
  }
  else { // altes Potential
    for (/*Pointer<Pedestrian>*/ ped=street->pedestrians; ped; ++ped) {
      for (int v=0; v<ped->popsize(); ++v) {
	if (ped()==this && w==v) continue; 
	Vector D = X[w] - ped->X[v];
	double d1 = D.length();
	Vector VT = ped->E0[v] * ped->V[v].length() * ped->delta; 
	Vector D2 = D-VT;
	double d2 = D2.length(); 
	if (d1<=rmax || d2<=rmax) {
	  bool inters = 0;
	  if (!flag[w].xray) {
	    for (/*Pointer<Obstacle>*/ obs = street->obstacles; obs; ++obs) {
	      if (inters |= obs->intersect(ped->X[v], X[w])) break;
	    }
	  }
	  if (!inters) {
	    double a = d1+d2, r = sqrt(a*a - VT*VT)/2.0;
	    u = pi * exp(-r/sigma); 
	    P(ped->clan,w) += u;     // Entropie
	    double gamma = u / ped->sigma * a/r;       
	    f += fprime = (D/d1 + D2/d2) * gamma;
	    if (F2 && nF) {
	      *F2 += fprime.x * fprime.x + fprime.x * fprime.y;
	      ++(*nF);
	    }
	  }
	}
      }
    }
  }
  // Entropyberechnen
  if (Nclans && P.N()*P.M()) {
    double Ptot = 0.0;
    for (c=0; c<Nclans; ++c) Ptot += P(c,w);
    if (Ptot>0.0) for (c=0; c<Nclans; ++c) P(c,w) /= Ptot;
  }
  // cout << f.x << '|' << f.y << endl; 
  return f;
}

Vector Pedestrian::Fped(int w, double *F2, int *nF, double *S) {
  // alt wird nicht mehr verwendet
  Vector f(0.0, 0.0), f_hat, fprime;
  Vector R, R_hat, X_hat, Y_hat;
  double r, gamma, u, y_hat;
  bool inters;
  int v, c;
  double P[Nclans];
  for (c=0; c<Nclans; ++c) P[c] = 0.0; // nP[c] = 0;}
  Pointer<Pedestrian> ped;
  Pointer<Obstacle> obs; 
  if (!flag[w].ellip) {
    for (ped=street->pedestrians; ped; ++ped) {
      for (v=0; v<ped->popsize(); ++v) {
	if ((ped()==this && w==v) || !ped->flag[v].visible) continue;
	R = X[w] - ped->X[v];
	r = R.length();
	if (r>rmax) continue;
	inters = 0;
	if (!flag[w].xray) {
	  for (obs = street->obstacles; obs; ++obs) {
	    if (inters |= obs->intersect(ped->X[v], X[w])) break;
	  }
	}
	if (inters) continue;
	// nichts hat sie aufgehalten
	// Y_hat = E0[w] * V[w].length() - ped->E0[v] * ped->V[v].length();
	// Y_hat = E0[w] -  ped->E0[v] * ped->V[v].length()/v0[w];
	// Y_hat = E0[w] -  ped->E0[v] * ped->v0[v]/v0[w];
	Y_hat = E0[w] -  (ped->E0[v] *(delta*ped->v0[v]/v0[w]));
	y_hat = Y_hat.length();
	if (y_hat>0.0) {
	  Y_hat /= y_hat;
	}
	else {
	  Y_hat = ped->X[v] - X[w];
	  y_hat = Y_hat.length();
	  if (y_hat>0.0) Y_hat /= y_hat;
	}
	X_hat = !Y_hat; // X_hat.x = - Y_hat.y; X_hat.y = Y_hat.x;
	X_hat /= X_hat.length();
	
	R_hat.x = X_hat * R; R_hat.y = Y_hat * R;
	// verkuerzung, verlaengerung
	gamma = 1.0 + V[w].length() * lam;
	if (R_hat.y < 0) R_hat.y /= (gamma*gamma);
	//else R_hat.y *= 1.0 + V[w].length() * lam;
	r = R_hat.length();
	u = pi * exp(-r/sigma); 
	c = ped->clan; P[c] += u;     // Negentropie
	gamma = 1 / sigma / r * u;
	f_hat =  R_hat * gamma; // X_hat * gamma + Y_hat * gamma; 
	f += fprime = X_hat * f_hat.x + Y_hat * f_hat.y;
	if (F2 && nF) {
	  *F2 += fprime.x * fprime.x + fprime.x * fprime.y;
	  ++(*nF);
	}
      }
    }
  }
  else { // altes Potential
    for (Pointer<Pedestrian> ped=street->pedestrians; ped; ++ped) {
      for (int v=0; v<ped->popsize(); ++v) {
	if (ped()==this && w==v) continue; 
	Vector D = X[w] - ped->X[v];
	double d1 = D.length();
	Vector VT = ped->E0[v] * ped->V[v].length() * ped->delta; 
	Vector D2 = D-VT;
	double d2 = D2.length(); 
	if (d1<=rmax || d2<=rmax) {
	  bool inters = 0;
	  if (!flag[w].xray) {
	    for (Pointer<Obstacle>obs = street->obstacles; obs; ++obs) {
	      if (inters |= obs->intersect(ped->X[v], X[w])) break;
	    }
	  }
	  if (!inters) {
	    double a = d1+d2, r = sqrt(a*a - VT*VT)/2.0;
	    u = pi * exp(-r/sigma); 
	    c = ped->clan; P[c] += u;     // Negentropie
	    double gamma = u / ped->sigma * a/r;       
	    f += fprime = (D/d1 + D2/d2) * gamma;
	    if (F2 && nF) {
	      *F2 += fprime.x * fprime.x + fprime.x * fprime.y;
	      ++(*nF);
	    }
	  }
	}
      }
    }
  }
  // Entropyberechnen
  if (S) {
    *S = 0.0;
    double Ptot = 0.0;
    for (c=0; c<Nclans; ++c) Ptot += P[c];
    if (Ptot>0.0) 
      for (c=0; c<Nclans; ++c) 
	*S += P[c]>0.0 ? P[c]/Ptot*log(P[c]/Ptot) : 0.0;
  }
  // cout << f.x << '|' << f.y << endl; 
  return f;
}

// void Pedestrian::clock() {
//   Walker::clock();
// }

Pedestrian::operator= (Pedestrian& ped) {
  char *p = (char*)&ped;
  char *q = (char*)this;
  int n = sizeof(Pedestrian);
  while (n-- ) *q++ = *p++;
}

int Pedestrian::mwrite(int fd) {
  int rc=0;

  rc += Walker::mwrite(fd);
  if (rc<0) return rc;

  if (fd!=-1) {
    rc += write_netbor4(fd,(double*) X0,popsize()*2)<<2;
    rc += write_netbor4(fd,Tneeded,popsize())<<2;
    rc += write_netbor4(fd,Dneeded,popsize())<<2;
    rc += write_netbor4(fd,width)<<2;
  }
  else {
    rc += (16*popsize())+4;
  }

  return rc;
}

int Pedestrian::mread(int fd, int num) {
  int rc=0,tmp,i;

  tmp=popsize();
  rc += Walker::mread(fd,num);
  if (rc==-1) return rc;

  if (popsize() > tmp) {
    delete [] Tneeded;
    delete [] Dneeded;
    Tneeded = new double[popsize()];
    Dneeded = new double[popsize()];
  }

  // now read the data of a pedestrian:
  if (num-rc<4*4*popsize()+4) return rc; // there are not enough data

  tmp=rc+4*4*(int)popsize()+4;
  rc += read_netbor4(fd,(double*) X0,(int)popsize()*2)<<2;
  rc += read_netbor4(fd,Tneeded,(int)popsize())<<2;
  rc += read_netbor4(fd,Dneeded,(int)popsize())<<2;
  rc += read_netbor4(fd,&width)<<2;

  // rc should be equal to tmp, otherwise this indicates an error.
  //  Don't adjust here any more. This MUST be done by the caller.
  //  Otherwise we might remove data of a superceding type.
  
  //  if(num-tmp) {
  //    // read pending bytes:
  //    // cerr<<"Warning: rc is "<<rc<<" should be "<<tmp<<'.'<<endl;
  //    char* buf;
  //    buf = new char [num-tmp];
  //    rc += ::read(fd,buf,num-tmp);
  //    delete [] buf;
  //  }
  
  return rc;
}

int Pedestrian::write(int stream) {
  int n = 0;
  for (int w=0; w<popsize(); ++w) {
    if (flag[w].visible) {
      /* WalkerRecordBlock wrb;
	 // erster Block
	 wrb.type = WRBdata;
	 wrb.continued = 1;
	 wrb.data.clan = clan;
	 wrb.data.x = X.x;
	 wrb.data.y = X.y;
	 wrb.data.vx = V.x;
	 wrb.data.vy = V.y;
	 wrb.data.v0 = v0;
	 wrb.data.from_gate = 0;
	 wrb.data.to_gate = 0;
	 int res = wrb.write(stream);
	 // zweiter Block
	 wrb.type = WRBdata2;
	 wrb.continued = 0;
	 wrb.data2.dummy = 0;
	 wrb.data2.x0 = X0.x;
	 wrb.data2.y0 = X0.y;
	 wrb.data2.tneed = Tneeded;
	 wrb.data2.lneed = Dneeded;
	 wrb.data2.width = width; 
	 return res + wrb.write(stream); */
    }
  }
  return n;
}


/* int Pedestrian::read(int stream) {
   WalkerRecordBlock wrb;
   wrb.continued = 1;
   int sum = 0;
   while (wrb.continued) {
   int res = wrb.read(stream);
   if (res<=0) return sum;
   sum += res;
   switch (wrb.type) {
   default:
   case WRBmisc:
   break;
   case WRBdata:
   clan = wrb.data.clan;
   X.x = wrb.data.x;
   X.y = wrb.data.y;
   V.x = wrb.data.vx;
   V.y = wrb.data.vy;
   v0  = wrb.data.v0;
   break;
   case WRBdata2:
   width = wrb.data2.a[0];
   break;
   case WRBdata3:
   break;
   }
   }
   return sum;
   } */

EvalPed::EvalPed() {
  /* ParameterDef d[] = { {"evalped", "1"}
     };
     if(!parameter) 
     parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
     else 
     parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
     */ 
  
  s = NULL;
  S = NULL;
  fitness = NULL;
  last_gate = NULL;
  stream = stream2 = NULL;
}

EvalPed::~EvalPed() {
  cout << "ciao evalped!\n";
}

void EvalPed::init() {
  Pedestrian::init();
  s = new Chromosome[popsize()];
  fitness = new Data[popsize()];
  last_gate = new int[popsize()];
  char *filename;
  get(parameter, "filename", &filename, " ");
  //if (!stream && *filename) 
  if (stream) fclose(stream);
  stream = fopen(filename, "w"); 
  get(parameter, "filename2", &filename, " ");
  //if (!stream2 && *filename) 
  if (stream2) fclose(stream2);
  stream2 = fopen(filename, "w"); 
  EvalPed::reset();
}

void EvalPed::reset(int z) {
  if (z<0) {
    parameter->write(stdout);
    Pedestrian::reset();
  // erzeuge Entscheidungs Chromosomen
    double l[3], u[3];
    double lambda, xi, deltat;
    get(parameter, "s_lambda", &lambda, 0.0); 
    get(parameter, "s_xi", &xi, 0.001); 
    get(parameter, "s_delta", &deltat, 0.0); 
    get(parameter, "s_lambda_min", &l[0], lambda); if (l[0]<0.0) l[0] = 0.0;
    get(parameter, "s_lambda_max", &u[0], lambda);
    get(parameter, "s_xi_min", &l[1], xi); if(l[1]<0.001) l[1] = 0.001;
    get(parameter, "s_xi_max", &u[1], xi);
    get(parameter, "s_delta_min", &l[2], deltat); if (l[2]<0.0) l[2] = 0.0;
    get(parameter, "s_delta_max", &u[2], deltat);
    Chromosome muster(3, l, u);
    int w;
    for (w=0; w<popsize(); ++w) {
      s[w] = muster;
      fitness[w].clear();
      last_gate[w] = -1;
    }
    Genotype::Parameters Sparam;
    get(parameter, "evol_a", &Sparam.a, 1.0);
    get(parameter, "evol_b", &Sparam.b, 1.0);
    get(parameter, "evol_prop", &Sparam.prop, 0.5);
    get(parameter, "evol_psc", &Sparam.psc, 0.0);
    get(parameter, "evol_pac", &Sparam.pac, 0.0);
    get(parameter, "evol_pum", &Sparam.pum, 0.0);
    get(parameter, "evol_pbm", &Sparam.pbm, 0.0);
    get(parameter, "evol_pnm", &Sparam.pnm, 0.0);
    
    if (S) delete S;
    S = new Genotype(s, int(popsize()), Sparam);
    get(parameter, "evol_time", &evol.T, 100.0);
    get(parameter, "evol_dt", &evol.Dt, 100.0);
    
    evol.epoch = 0;
    evol.best = evol.average = 0.0;
    S->reproduction(evol.epoch);
    for (w=0; w<popsize(); ++w) {
      cout << "chromosome " << w << ": " << (*S)[w] << endl;
    }
    
  }
  else { 
    Pedestrian::reset(z);
  }
}

int  EvalPed::update() {
  double fx, fy, _fp, Fx, Fy, v, weight, dx0, dy0, r0, ex0, ey0;
  Vector FF, f, Xd, dV;
  double m, m_rezi, l;
  int i, j, k, n;
  double F2, Negentropy;
  int nF;

  Pointer<Pedestrian> p;

  int w;
  
  switch (objecttask) {
  case SET_VALUES:
  case RESET:
    // Anpassen der Parameter, die von anderen Objekten abhaengen
    // if  (vmax > floor->vmax) vmax = floor->vmax;  
    for (w=0; w<popsize(); ++w) flag[w].reset = 1;
    /* Zeichnen */
    //  draw_flag |= CuDontDrawTrace;
    
    objecttask = RUN;
    // no break!
  case RUN:
    //extern FILE *Logfp;
    // evolution
    if (floor->T >= evol.T) {
      int i;
      double fit;
      for (i=0; i<popsize(); ++i) {
	fit = fitness[i].N() ? fitness[i].mean() : 0.0;
	(*S)[i].fitness(fit);
	// ausgabe
	if (stream2) fprintf(stream2, "%lf %lf %lf\t%lf\n", 
			     (*S)[i][0], (*S)[i][1], (*S)[i][2], fit); 
      }
      if (stream2) fflush(stream2);
      S->survival();
      evol.best = S->best().fitness();
      evol.average = 0.0;
      for (i=0; i<popsize(); ++i) evol.average += s[i].fitness();
      evol.average = popsize()? evol.average/popsize(): 0.0;
      if (stream) {
	cout << "nextgen " << evol.epoch << endl;
	fprintf(stream, "%d\t%lf\t%lf\t", evol.epoch, evol.best, evol.average);
	fprintf(stream, "%lf %lf %lf\t%lf\n", 
		S->best()[0], S->best()[1], S->best()[2], fit);
	fflush(stream);
      }
      evol.epoch++;
      evol.T += evol.Dt;
      S->reproduction(evol.epoch); // strategie der FG wird veraendert !!!
      for (i=0; i<popsize(); ++i) fitness[i].clear();
    }
						   
    n = 0;
    for (w=0; w<popsize(); ++w) {
      // falls Fussgaenger noch im Rennen 
      if (flag[w].reset) {
	if (dest[w]) Texpected[w] = dest[w]->dest(X[w]).length()/v0[w];
	else Texpected[w] = r0;
	Tneeded[w] = 0.0;
	Dneeded[w] = 0.0;
	Ttotal[w] = 0.0;
	Ttotex[w] = 0.0;
	Dtotal[w] = 0.0;
	flag[w].reset = 0;
	for (int i=0; i<NY; ++i) Y(i,w).clear();
	for (int j=0; j<Nclans; ++j) Pac(j,w) = 0.0;
      }
      if (flag[w].active) { // (flag & PedActive) {      
	if (T[w] >= floor->T) {
	  //Fussgaenger in Warteposition
	  n++;
	}
	else {
	  flag[w].visible = 1;
	  //      draw_flag &= ~CuDontDrawTrace;
	  
	  // Entfernung zum Ziel 
	  //abstand = floor->route.distance(orig, dest);
	  //Xd = X0 - X;
	  if (dest[w]) {
	    //cout << "ped " << w << " Nalt " << dest[w]->Nalt();
	    //cout << " last " << last_gate[w];
	    Gate *gat = select(dest[w], X[w], v0[w], (*S)[w].x, 
			       &Xd, &last_gate[w]);
	    //cout << " new " << last_gate[w] << endl;
	    gat->velocity(Xd,V[w]);
	    X0[w] = X[w] + Xd;
	  }
	  else {
	    Xd = X0[w] - X[w];
	  }
	  r0 = Xd.length();
	  if (r0<=0.0) /* goto PED_ABB*/ ; //evtl. Abbruch, wenn Ziel erreicht
	  E0[w] = r0>0.0? Xd/r0: Vector(0.0,0.0);
	  v = V[w].length();     
	
	  //F = street->F(X, v>0.0?V/v:Vector(0.0,0.0), weightShift);
	  
	  // START
	  FF = Vector(0.0,0.0);
	  
	  // Ground-Potential
	  for (Pointer<Ground> gr=grounds; gr; ++gr) {
	    FF += gr->F(X[w]);
	  }
	  // Hindernis
	  for (Pointer<Obstacle> o=street->obstacles; o; ++o) {
	    f = o->F(X[w]);
	    //if ((f*E0) > 0.0) f*= weightShift;
	    FF += f; // beta * ?? 
	    // maximum:  if (f.length() > F.length()) F = f;
	    // summe: Fres += Ftemp;
	    // minimum: if (Ftemp.length() < r && Ftemp*Ex < 0.0) { 
	    //            Fres = Ftemp;
	    //            r = Ftemp.length();
	    //          }
	  }	
	  
	  // Fussgaenger-Abstossung 
	  FF += Fped(w, &F2, &nF, &Negentropy);

	  // Fluktuation
	  if (mu>0.0) {
	    double f2 = FF*FF;
	    if (f2>0.0) {
	      double fluct = rndgauss(0.0, mu)*(FF*E0[w])/f2;
	      FF += Vector(fluct*-FF.y, fluct*FF.x);
	    }
	  }
	  
	  // Geschwindigkeitsaenderung
	  V[w] += ((E0[w]*v0[w] - V[w])/tau[w] + FF) * floor->Dt;
	  
	  // Theta-Funktion, Geschwindigkeit nach oben abschneiden 
	  v = V[w].length();     
	  if (v > vmax[w] && v > 0.0) {
	    V[w] = V[w] * vmax[w]/v;
	    v = vmax[w];
	}
	
	  // Orts-Aenderung
	  //Xp = X;
	  itail[w] = (itail[w]+1)%Ntail;
	  Xtail[w][itail[w]] = X[w];
	  X[w] += V[w] * floor->Dt;
	
	  // Markierungen
	  for (Pointer<Marker> mar=markers; mar; ++mar) {
	    //mar->stamp(X, (v-v0)*floor->Dt);
	    mar->stamp(X[w], V[w], v0[w]);
	  }
	
	  Dneeded[w] += V[w].length() * floor->Dt; //Dtotal  += v * floor->Dt;
	  // Timer 
	  T[w] = floor->T;
	  Tneeded[w] += floor->Dt;	//Ttotal  += floor->Dt;
	  
	  if (*street>=_CORRIDOR) {
	    ((Corridor*)street)->data(clan, X[w], v, v0[w], 0.0, 0.0);
	  }
	  
	  // Teste ob ein Tor durchschritten wurde, wenn ja: Neuer next.
	  
	  // Abbruchbedingung fuer Fussgaenger 
	  //if (dest && dest->dest(X).length() < V.length() * floor->Dt) {
	  if (dest[w] && dest[w]->passed(X[w], Xtail[w][itail[w]])) { 
	    // richtung hat sich geaendert.
	    // Tor erreicht, Zeitnehmer rausschreiben, naechstes Tor 
	    /* FORMAT NR T V0 Tex Tneed Dneed */
	    //	if (Logfp) fprintf(Logfp, "%d %lf %lf %lf %lf %lf \n", 
	    //	   dest->nr, T, v0, Texpected, Tneeded, Dneeded);
	    Ttotal[w] += Tneeded[w];
	    Ttotex[w] += Texpected[w];
	    Dtotal[w] += Dneeded[w];
	    //  printf("d=%lf t=%lf dtotex=%lf ttotex=%lf\n" ,Dtotal, Ttotal, Ttotex*v0, Ttotex); 
	    (*S)[w].fitness(Texpected[w]/Tneeded[w]);
	    orig[w] = dest[w];
	    ++dest[w];
	    last_gate[w] = -1;
	    if (dest[w]) { // neues Ziel definieren
	      // neues Ziel, Statistik 
	      Texpected[w] = dest[w]->dest(X[w]).length()/v0[w];
	      Tneeded[w] = 0.0;
	      Dneeded[w] = 0.0;
	      n++;
	    } 
	    else if (flag[w].periodic) { // kann wiedergeboren werden 
	      // GesamtZeit
// 	      floor->journey(clan, v0[w], Dtotal[w], Ttotal[w], 
// 			     Ttotex[w]*v0[w], Ttotex[w]); 
	      reset(w);
	      n++; // weiter bei Reincarnation 
	    }
	    else {
// 	      floor->journey(clan, v0[w], Dtotal[w], Ttotal[w],
// 			     Ttotex[w]*v0[w], Ttotex[w]); 
	      V[w] = Vector(0.0, 0.0);
	      flag[w].active = 0; // flag &= ~PedActive;
	    }
	  }
	  else n++; // increment counter n, continue for n > 0 
	}
      } // if Active
    } // for w
    break;
  default:
      break;
  } // switch
  return(n);
}

Gate* EvalPed::select(Gate* pg, Vector &X, double v0, double *pp,
		     Vector *Xd, int *k) {
  // entscheide fuer welche Tuer
  Gate &g = *pg;
  double lambda = pp[0];
  double xi = pp[1];
  double delta = pp[2];
  int n = g.Nalt();
  if (n>1) {
    Vector S, Es;
    double s, tt, dt;
    double t[n];
    double p[n];
    Vector D[n];
    test(cout << " ");
    for(int l=0; l<n; ++l) {
      S = g[l].Q - g[l].P; S = !S; s = S.length(); 
      D[l] = g[l].dest(X);
      Es = D[l]*S < 0.0 ? -S/s : S/s;
      tt = g[l].vel.N() ? (v0 - g[l].vel.mean()*Es) / v0 : 0.0;
      if (tt<0.0) tt = 0.0;
      dt = l==*k? delta: 0.0; // Kronecker * deltaT
      // t
      t[l] = D[l].length()/v0 + tt*lambda; 
      p[l] = exp(-(t[l]-dt)/xi);
      test(cout << t[l] << ':' << p[l] << ", ");
    }
    *k = irndsel(p,n);
    *Xd = D[*k];
    return &g[*k];
  }
  else {
    *k = 0;
    *Xd = g.dest(X);
    return pg;
  }
}


/* End of File */















