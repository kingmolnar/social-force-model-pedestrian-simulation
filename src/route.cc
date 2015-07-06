
#include <stdio.h>
#include <assert.h>
#define NTEST
#include "test.h"
#include "assert.h"

#include "street.hh"
#include "pedestrian.hh"
#include "mall.hh"
#include "route.hh"
 

//inline static double sq(double X) {return X*X;}

// Vertex
/* Vertex::Vertex() {
  ParameterDef d[] = { {"p", "0.0|0.0"},
		       {"funct", "1"},
		       {"color", "./."}
		     };
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  objecttype = _VERTEX;
  parent = 0;
}

void Vertex::init() {
  switch (parameter->nv) {
  case 1:
    P = parameter->v[0];
    break;
  default:
    parameter->get("p", &P);
    break;
  }
  parameter->get("funct", &funct);
  parameter->get("color", &color);
}

int Vertex::refup(SimulObject *obj) {
  int good = 0;  
  switch (obj->objecttype) {
  case _ENVIRONMENT:
  case _STREET:
    environments.append(obj);
    good = 1;
    break;
  default:
    break;
  }
  return good;
}

Vector Vertex::dest(Vector X) {
  return P-X;
}
*/

// Gate
Gate::Gate() {
  ParameterDef d[] = { {"p", "0.0|0.0"},
		       {"q", "0.0|0.0"},	     
		       {"funct", "1"},
		       {"color", "./."}
		     };
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  objecttype = _GATE;
  parent = 0;
}

void Gate::init()
{
  switch (parameter->nv) {
  case 2:
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
}

int Gate::refup(SimulObject *obj) {
  int good = 0;  
  switch (obj->objecttype) {
  case _ENVIRONMENT:
  case _STREET:
    environments.append(obj);
    good = 1;
    break;
  default:
    break;
  }
  return good;
}

int Gate::simul(Pedestrian *p)
{
  /*   Mall *mall = p->mall;
       int i;
       double Trel, Drel;
       i = int((p->v0 - mall->vmin)/(mall->vmax - mall->vmin) * numberv);
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

Vector Gate::dest(Vector X)
{
  double a, b, cos_a, cos_b, c, cx, cy, d, dx, dy, rmin;
  double x0x, y0y, x1x, y1y, mp, mq;
  Vector E, D, Rp, Rq;
  
  if (P == Q) {
    E = P - X;
  } 
  else {
    D = P - Q; //dx = gat->x1-gat->x0;    dy = gat->y1-gat->y0;
    d = D.length();
    dx /= d;
    dy /= d;
    Rp = P - X; mp = Rp.length();
    Rq = Q - X; mq = Rq.length();
    
    cos_a = (D * (-Rp)) / mp;
    cos_b = (D * (-Rq)) / mq;
    
    if (cos_a <= 0.0) {
      E = Rp; // *ex = gat->x0 - x; *ey = gat->y0 - y; rmin = m0;
    }
    else if (cos_b <= 0.0) {
      E = Rq; // *ex = gat->x1 - x; *ey = gat->y1 - y; rmin = m1;
    }
    else {
      // E  =  Rp; // *ex = gat->x0 - x;  *ey = gat->y0 - y;
      d =  cos_a * mp;
      E = Rp + (D*d); // *ex += d * dx; *ey += d * dy; rmin = mass(*ex, *ey);
    }
  }
  return(E);
}

Vector Gate::dest(Gate *gat) {
  Vector R0 = dest(gat->P);
  Vector R1 = dest(gat->Q);
  if (R0.length() < R1.length()) return R0;
  else return R1;
}

/* 
// Alternative
Alternative::Alternative() {
}

//  Alternative::~Alternative() {}

void Alternative::init() {
}

int Alternative::refup(SimulObject*) {
}

int Alternative::refdown(SimulObject*) {
}

Vector Alternative::dest(Vector& X) {
}

Vector Alternative::dest(Gate *gat) {
}
*/

/* end of file */











