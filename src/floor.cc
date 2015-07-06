#define NTEST
#include "test.h"
#include <assert.h>

#include "list.hh"
#include "vector.hh"

#include "pedestrian.hh"
#include "street.hh"
//#include "route.hh"
//#include "attraction.hh"
#include "ground.hh"
#include "floor.hh"
#include "movie.hh"


SimulObject* SimulCreatorFloor::create(SimulObjectType y) {
  SimulObject* obj = NULL;
  switch (y) {
  case _UNDEF:
  default: 
    printf("unknown object-type.\n");
    obj = NULL;
    break;
  case _FLOOR:
    obj = new Floor;
    break;
  } // end switch
  return obj;
}

SimulObjectType SimulCreatorFloor::type(char *token) {
  SimulObjectType t = _UNDEF;
  if (strcasecmp(token, "floor")==0) t = _FLOOR;
  return t;
}

Floor::Floor()
{
  // create parameters
  // ParameterDef d[]; //  = { };
  prffp = NULL;
  journ = NULL;
  prog = NULL;
  if (!parameter)
    parameter = new Parameter(); //(d, sizeof(d)/sizeof(ParameterDef));
  // else  parameter->insert(d, sizeof(d)/sizeof(ParameterDef));
  //  objecttype = _FLOOR;
  Nvars = 6;
  progM = 5;
  progMIN = 100;
}

// Floor::Floor(FILE *fp) {  read(fp);  init();}

Floor::~Floor() {
  if (prog) {
    for (int p=0; p<Nvars; ++p) 
      if (prog[p]) delete prog[p];
    delete prog;
  }
  if (prffp) {
    //fputc('$', prffp);
    fclose(prffp);
  }
  /*   m = floor->succ;
	 m->prec = floor->prec;
	 floor->prec->succ = m; */
  /* vorher sollte aber aufgeraeumt werden! */
  // Street_Delete(floor->street);
  // Pedestrian_Delete(floor->ped);
  //  Gate_Delete(floor->gat);
}

void Floor::init() {  
  WithLogfile::init();
  int st = 1;
  // cout << "init floor\n";
  // Variablen festlegen
  parameter->get("xmin", &Xmin);
  parameter->get("xmax", &Xmax);
  parameter->get("dx", &Dx);
  parameter->get("vmin", &vmin);
  parameter->get("vmax", &vmax);

  parameter->get("eps_p", &epsilon_P);
  parameter->get("eps_b", &epsilon_B);
  
  parameter->get("del", &Delta);
  //{"beta", "1.0"}, {"sig_b", "0.6"},{"eps_b", "0.0001"},
  get(parameter, "tau", &Tau, 1.0);
  get(parameter, "dt", &Dt, 0.1);
  // {"snapshot", "3"}, {"trace", "10"}, {"tresh", "0.01"}, {"weight", "0.2"} 

  //  test(printf("done.\n\n"));
  get(parameter, "nparts", &Nparts, 1);
  if (Nparts<1) Nparts = 1;
  get(parameter, "nclans", &Nclans, 1);
  if (Nclans<1) Nclans = 1;
  if (journ) delete journ;
  journ = new arrlst<Data*>(NULL, 4*(Nparts+1));
  char *perfname = 0;
  get(parameter, "performance", &perfname, "");
  if (perfname && *perfname) prffp = fopen(perfname,"w");
  prog = new PrognosisPointer[Nvars];
  for (int p=0; p<Nvars; ++p) {
    prog[p] = new Prognosis(progM, progMIN);
  }
  T = 0.0;
}  

int Floor::ref(SimulObject* obj, SimulObjectType type) {
  int good = 0;  
  switch (type) { 
  case _STREET:
    streets.append(obj);
    good = 1;
    break;
  case _ATTRACTION:
    attractions.append(obj);
    good = 1;
    break;
  case _GATE:
    gates.append(obj);
    good = 1;
    break; 
#ifdef _ROUTE_HH_
  case _NETWORK:
    if (!network) {
      network = (Network*)obj;
      good = 1;
    }
    else good = 0;
    break; 
#endif
  case _GROUND:
    grounds.append(obj);
    good = 1;
    break;
  case _MARKER:
    markers.append(obj);
    good = 1;
    break;
  case _PEDESTRIAN:
    pedestrians.append(obj);
    good = 1;
    break;
  case _FLOOR:
  case _POLYGON:
  case _CIRCLE:
  case _UNDEF:
  default:
    good = 0;
    break;
  }
  test(printf("Floor::append > type=%d good=%d\n", obj->objecttype, good));
  return good ? 1 : WithLogfile::ref(obj, type);
}

void Floor::calc()
{
#ifdef _STREET_HH_
  Pointer<Street> s;
  for (s=streets; s; ++s) {
//    s->calc();
  }
#endif
} 


Data* Floor::journey(int cl, int part) {
  if (cl>=Nclans) Nclans = cl+1; 
  if (journ) {
    Data* dp = (*journ)[cl*(Nparts+1)+part];
    if (dp==NULL) {
      dp = (*journ)[cl*(Nparts+1)+part] = new Data(Nvars);
    }
    return dp;
  }
  else return new Data(Nvars);
}

Data* Floor::journey(int cl, double v0) {
  double dx = (vmax-vmin)/Nparts;
  int i;
  for (i=1; i<Nparts; ++i) {
    if (v0<vmin+dx*double(i)) break;
  }
  return journey(cl, i);
}


// Data* Floor::journey(int cl, double v0, double td, double tt, double ed,
// 		     double et) {  
//   Data* j = journey(cl+1, v0);
//   if (tt>0.0 && ed>0.0) {
//     double x[Nvars];
//     x[0] = td; // distance
//     x[1] = tt; // time
//     x[2] = td/tt; // velocity
//     x[3] = 100 * td/ed; // strain
//     x[4] = 100.0 * et/tt; // effectiveness

// Data* Floor::journey(int cl, double v0, double eta, double zeta, 
// 		     double neg, double xi) {

Data* Floor::journey(int cl, double v0, double *x_, int nvars) {
  Data* j = journey(cl+1, v0);
  double x[Nvars];
  if (nvars<Nvars) {
    int i;
    for (i=0; i<nvars; ++i) x[i] = x_[i];
    for (; i<Nvars; ++i) x[i] = 0.0;
  }
  else {
    int i;
    for (i=0; i<Nvars; ++i) x[i] = x_[i];
  }
  // clan
  j->data(x);                // Veloc 
  journey(cl+1, 0)->data(x); // Total 
  // all
  journey(0, v0)->data(x);   // Veloc
  Data *j0 = journey(0, 0);
  j0->data(x);    // Total
  if (j0->N()) {
    double a, b, v;
    double epsilon = 0.000000001;
    for (int p=0; p<Nvars; ++p) {
      a = double(j0->N());
      b = j0->mean(p);
      v = j0->var(p);
      if (a>10.0 && fabs(b*v)>epsilon) { 
	prog[p]->data(a, b, v);
	// printf("prog %d %lf %lf %lf\n", p, 
	//        double(j0->N()),j0->mean(p), j0->std(p));
      }
    }
  }
  return j;
}


// indizierung des Datablocks
void Floor::log() {
  if (logfp) {
    int p, k;
    int z;
    // double *zeiger;
    fprintf(logfp, "time %8.2lf\n", T);
    // v0-Zeile
    fprintf(logfp, "v0\t");
    double dx = (vmax-vmin)/Nparts;
    for (k=0; k<Nparts; ++k) fprintf(logfp, "%16.4lf", vmin+dx*double(k));
    fprintf(logfp, "      TOTAL\n\n");
    //              01234567890. 
    // clans
    char lab[9] = {""};
    char B[] = "EVCGST__DTVSE";
    B[Nvars] = 0;
    for (int cc=1; cc<=Nclans; ++cc) {
      register int c = cc%Nclans;
      if (c) fprintf(logfp, "clan %5d\n", c-1);
      else   fprintf(logfp, "TOTAL\n");
      int NN[Nparts+2];
      //  double *Feld[Nparts+2];
      for (p=0; p<Nparts+1; ++p) {
	NN[p] = journey(c, p)->N();
      }
      // v0-Zeile
      //fprintf(logfp, "v0\t");
      //double dx = (vmax-vmin)/Nparts;
      //for (k=0; k<Nparts; ++k) fprintf(logfp, "%16.4lf", vmin+dx*double(k));
      //fprintf(logfp, "      TOTAL\n");
      //              01234567890. 
      // Anzahl
      //fprintf(logfp, "%s\t", lab);
      fprintf(logfp, "N\t");
      for (p=1;p<=Nparts+1;++p) fprintf(logfp,"%11d     ", NN[p%(Nparts+1)]);
      //                                           .0123
      fprintf(logfp, "\n");
      for (z=0; z<Nvars; ++z) {
	// mean value
	fprintf(logfp, "%c\t", B[z]);
	for (p=1; p<=Nparts+1; ++p) { 
	  register int pp = p%(Nparts+1);
	  if (NN[pp]) fprintf(logfp, "  %13.6le ", journey(c, pp)->mean(z));
	  else fprintf(logfp, "   -.-          ");
	  //                   --+0.012345e+01-
	}
	fprintf(logfp, "\n");
	// standard derivation/variance
	//fprintf(logfp, "std(%c)\t  ", B[z]);
	fprintf(logfp, "\t", B[z]);
	for (p=1; p<=Nparts+1; ++p) { 
	  register int pp = p%(Nparts+1);
	  //if (NN[pp]) fprintf(logfp, "  (%11.4le)", journey(c, pp)->var(z));
	  if (NN[pp]) fprintf(logfp, "  (%11.4le)", journey(c, pp)->std(z));
	  else fprintf(logfp, " ( -.-         )");
	  //                   -(+0.012345e+01)
	}
	fprintf(logfp, "\n");
      }
      //ende
      fprintf(logfp, "\n");
    } // clan
    
    // letzte Zeile
    Data *j = journey(0,0);
    if (j->N()) {
      fprintf(logfp, "N = %d T = %lf\n", j->N(), T);
      //      fprintf(logfp, "");
      for (z=0; z<Nvars; ++z) {
	fprintf(logfp, "   %c  = %11.4le (%11.4le)", 
		B[z], j->mean(z), j->std(z));
      }
      fprintf(logfp, "\n");
      if (prog) {
	for (z=0; z<Nvars; ++z) {
	  prog[z]->prognosis();
	  if (prog[z]->valid()) {
	    fprintf(logfp, " ");
	    int i;
	    for (i=0; i<progM; ++i) {
	      fprintf(logfp, "   %c%1d = %11.4le;",B[z], i, prog[z]->c(i));
	      //              ---
	    }
	    fprintf(logfp, "\n#");
	    for (i=0; i<progM; ++i) {
	      fprintf(logfp, "       (%11.4le)", sqrt(prog[z]->cvar(i)));
	      //              ---X9 = +0.---
	    }
	    fprintf(logfp, "\n");
	  }
	}
	fprintf(logfp, "\n");
      }
    } 
    fflush(logfp);
  }
  if (prffp) { // Performance file
    char B[] = "evcgst_____";
    Data *j = journey(0,0); 
    if (j->N()) {
      fprintf(prffp, "TNEVCGST\t%8.2lf %8d", T, j->N());
      for (int z=0; z<Nvars; ++z) {
	fprintf(prffp, " %13.6le", j->mean(z));
      }
      fprintf(prffp, "\n"); 
      
      if (prog && prog[0]) {
	int z;
	double pp; 
	fprintf(prffp, "prog\t\t%8.2lf %8d", T, j->N());
	for (z=0; z<Nvars; ++z) {
	  pp = prog[z]->c();
	  if (prog[z]->valid()) 
	    fprintf(prffp, " %13.6le", pp);
	  else fprintf(prffp, "  -.-         ");
	  //                    +0.012345e+01
	}
	fprintf(prffp, "\n"); 
      }
      fprintf(prffp, "\n"); 
    }
    fflush(prffp);
  }
}

// int Floor::read(FILE *fp){  int n = 0;  return n;}
// int Floor::write(FILE *fp){if (fp == NULL) return(0); else {for (Pointer<Pedestrian> p=pedestrians; p; ++p) {fprintf(fp, "p");}}  return(0);}

int Floor::mwrite(int fd) {

  int subchannels=0;
  register int i;
  int rc=0;

  // selection of which objects to write out
  subchannels += pedestrians.N()?1:0; Pointer<Pedestrian> ped;
  //subchannels += streets.N()?1:0; Pointer<Street> str;	
  //subchannels += gates.N()?1:0; Pointer <Gate> gat;
  //subchannels += grounds.N()?1:0; Pointer <Ground> gro;
  //subchannels += markers.N()?1:0; Pointer <Marker> mar;
  //subchannels += attractions.N()?1:0; Pointer <Attraction> att;

  rc+=(fd==-1)?2:write_netbor2(fd,&subchannels)<<1;

  rc+=write_object(fd,_PEDESTRIAN, ped, pedestrians );
  //rc+=::write_object(fd,_STREET, str, streets);
  //rc+=::write_object(fd,_GATE, gat, gates);
  //rc+=::write_object(fd,_GROUND, gro, grounds);
  //rc+=::write_object(fd,_MARKER, mar, markers);
  //rc+=::write_object(fd,_ATTRACTION, att, attrachtions);
  
  return rc;
}

int Floor::mread(int fd, int num)
{

  int subchannels,SimObjTyp,SimObjNum;
  register int i,j;
  int rc=0;

  //read number of subchannels
  rc+=read_netbor2(fd,&subchannels)<<1;

  //cerr<<" Reading "<<subchannels<<" subchannel(s)."<<endl;

  //read in all subchannels
  for (i=1; i<=subchannels; ++i)
  {
    rc+=read_netbor2(fd,&SimObjTyp)<<1;
    rc+=read_netbor2(fd,&SimObjNum)<<1;
    Pointer<Pedestrian> ped;
    Pointer<Walker> dummy;	//any type already used
    switch (SimObjTyp)
    {
      case _PEDESTRIAN:
	//cerr<<" Reading "<<SimObjNum<<" Pedestrians."<<endl;
	rc+=::read_object(fd,_PEDESTRIAN,&ped,&pedestrians,SimObjNum);
	break;
      default:
	List DummyList;
	//cerr<<" Discarding "<<SimObjNum<<" objects of type "<<SimObjTyp<<" ."<<endl;
	rc+=::read_object(fd,_DUMMY,&dummy,&DummyList,SimObjNum);
      }
  }
  return rc;
}

int Floor::update() {
  int n = 0;
  Pointer<Ground> gro;
  for (gro=grounds; gro; ++gro) {
    gro->decay();
  }
  Pointer<Gate> gat;
  for (gat=gates; gat; ++gat) {
    gat->vel.clear();
  }
  Pointer<Street> str;
  // for (str=streets; str; ++str) {
  // if (*str>=_CORRIDOR) ((Corridor*)str)->prologue();
  // }
  
  for (Pointer<Pedestrian> ped=pedestrians; ped; ++ped) {
    n += ped->update();
  } /* ped loop */
  
#ifdef _ATTRACTION_HH_
  for (str=streets; str; ++str) {
    for (att=str->attractions; att; ++att) {
      n += att->update();
    } /* att loop */      
  } /* street loop, ein Zeitschritt ist zuende */
#endif
  // for (str=streets; str; ++str) {
  //  if (*str>=_CORRIDOR) {
  // ((Corridor*)str)->epilogue();
  //    ((Corridor*)str)->report();
  //   }
  //  }
	  
  for (gro=grounds; gro; ++gro) {
    gro->adjust();
  } 
  for (Pointer<Marker> mar=markers; mar; ++mar) {
    mar->update();
  } 
  for (gat=gates; gat; ++gat) {
    gat->log();
  }
  T += Dt;    
  return n;
}









