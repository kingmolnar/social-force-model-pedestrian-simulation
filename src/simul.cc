#define NTEST


#include "test.h"
#include <assert.h>
#include "simul.hh"
#include <iostream.h>


Simul *root;

SimulObject *newSzeneFenster();
SimulObject *newVideoFenster();
SimulObject *newReplayFenster();
SimulObject *newSceneryPlot();
SimulObject *newConturPlot();

extern "C" {
  //  int strcasecmp(char*, char*);
  //  char *strncpy(char*, char*, unsigned int);
  char *index(char *s, char c);
}

#ifdef NEED_SETENV
extern "C" {
  char*	strcat(char*, const char*);
}
// needs to be put in its own object file (grmbl)
int setenv(char* const name, char* const value, const int overwrite)
{
  // check if it already exists. If it does and overwrite=0,
  // do not overwrite the old value and return 0:

  if ( (! overwrite) &&  getenv(name) )	return (0);

  // get space for new environment variable.
  // +1 for '=', +1 for \0

  char* newEnv;
  if ( ! (newEnv = new char[ strlen(name)+strlen(value)+2 ]) )
    return ( -1 );	// 0 was returned by new --> setenv failed.

  strcat(strcat(strcpy(newEnv,name),"="),value);

  return ( putenv(newEnv) ? 0 : -1 );
}
#endif
 
#define REF_AT_INIT

static int refref(SimulObject *up, SimulObject *down) {
  // Alternative Zwischenspeichern
  up->refs.append((ListElement*)down);
  // down->refs.append((ListElement*)up); 
  // das aktuelle Object speichert seine Refs
  //printf("refref up=%d down=%d\n", up->objecttype(), down->objecttype()); 
#ifndef REF_AT_INIT
  int k, good;
  SimulObjectType type;
  if (up==NULL || down==NULL ) goto erfolglos;
  k = 0;
  good = 0;
  type = down->obj->objecttype(k);
  while (!good && type !=_UNDEF) {
    good = up->obj->ref(down->obj, type);
    type = down->obj->objecttype(++k);
  }
  if (!good) goto erfolglos;
  
  k = 0;
  good = 0;
  type = up->obj->objecttype(k);
  while (!good && type !=_UNDEF) {
    good = down->obj->ref(up->obj, type);
    type = up->obj->objecttype(++k);
  }
  if (!good) goto erfolglos;
  return 1;
 erfolglos:
  return 0;
#else
  return 1;
#endif
}

SimulObject::SimulObject() {
  parent = NULL;
}


void SimulObject::init() {
  //printf("SimulObject::init()\n");
  WithParameter::init();
  //refs.append(parent);
  int k, good; 
  SimulObjectType type;
  // parent 
  SimulObject *par = parent;
  while (par){
    k = 0;
    good = 0;
    type = par->objecttype(k);
    while (!good && type !=_UNDEF) {
      good = ref(par, type);
      type = par->objecttype(++k);
    }
    par = par->parent;
  } 
  // references   
  for (Pointer<SimulObject> x=refs; x; ++x) {
    //printf("init-ref up=%d down=%d\n", objecttype(), x->objecttype());
#ifdef REF_AT_INIT
    k = 0;
    good = 0;
    type = x->objecttype(k);
    while (!good && type !=_UNDEF) {
      good = ref(x(), type);
      type = x->objecttype(++k);
    }
    /* k = 0;good = 0; type = objecttype(k); while (!good && type !=_UNDEF) {good = x->ref(this, type);type = objecttype(++k);} */
#endif
  }
}

int SimulObject::ref(SimulObject *obj, SimulObjectType t) {
  return 0;
}

SimulObject* SimulObject::clone() {
  SimulObject* x; // = CreateObject(objecttype(0));
  x->parameter = parameter;
  x->parent = parent;
  x->refs = refs; 
  return x;
}

int SimulObject::operator>=(const SimulObjectType t) {
  SimulObjectType ot;
  int k = 0; 
  do {
    ot = objecttype(k);
    if (ot==t) return 1;
    k++;
  } while (ot!=_UNDEF);
  return 0;
}
    

TextFile::TextFile(char *n, char *m) {
  name = new char[strlen(n)+1];
  strcpy(name, n);
  mod= new char[strlen(m)+1];
  strcpy(mod, m);
  stream = fopen(name, mod);
  if (stream) flag = 1;
  else flag = 0;
  li = 0;
  co = 0;
  buffer = new char[buffersize];
  bufferind = 0;
}

TextFile::~TextFile() {
  if (stream) fclose(stream);
  if (name) delete name;
  if (buffer) delete buffer;
};


void TextFile::reopen() {
  if (stream) fclose(stream);
  li = co = 0;
  if (name && mod) {
    stream = fopen(name, mod);
    if (stream) flag = 1;
    else flag = 0;
  }
}

static const int TOKENLENGTH = 512;

void TextFile::token(char *tk) {
  int n = 0;
//  static FILE *altstr = NULL;
  static int c = 0; // fgetc returns int
  char *t = tk;
  enum { kommentar, weiter, zeichenkette} st = weiter;
  // in der Routine fehlt noch die Behandlung von Zeichenketten!
  
  while (n<TOKENLENGTH) {
    if (!c) {
      c = fgetc(stream);
      buffer[bufferind] = c;
      bufferind = (bufferind+1)%buffersize;
      switch (c) {
      case '\n': case EOF:
	li++;
	co = 0;
	break;
      default:
	co++;
	break;
      }
    }
    switch (st) {
    case weiter:
      switch (c) {
      case ' ': case '\t': case '\n':
	if (n==0) c = 0;
	else { // token abschliessen und zeichen wegwerfen
	  c = *t = 0;
	  return;
	}
	break;
      case '#': // Kommentar einleiten bis Zeilenende
	c = 0;
	st = kommentar;
	break;
      case '"': // Zeichenkette einleiten bis "
	*t++ = '\''; n++;
	c = 0;
	st = zeichenkette;
	break;
      case EOF: case ';': /* case ',': */
      case '=': case '[': case ']':
      case '{': case '}': case '<': case '>': case '^': case ':':
	// einzelnes Zeichen
	if (n==0) {
	  *t++ = c;
	  c = *t = 0;
	  return;
	}
	else { // token abschliessen und zeichen behalten
	  *t = 0;
	  return;
	}
      default:
	*t++ = c; n++;
	st = weiter;
	c = 0;
	break;
      } // end switch (c)
      break;
    case kommentar:
      switch (c) {
      case '\n': case EOF: 
	if (n==0) {
	  if (c=='\n') c = 0; // nur eine kommentarzeile, weiterlesen
	  st = weiter;
	}
	else { // token abschliessen und zeichen wegwerfen
	  c = *t = 0;
	  return;
	} 
      default:
	c = 0;
	break;
      }
      break;
    case zeichenkette:
      switch (c) {
      case EOF: // fehler
	c = *t = 0;
	return;
      case '"':
	c = *t = 0;
	return;
      default:
	*t++ = c; n++;
	st = zeichenkette;
	c = 0;
	break;
      }
      break;
    } // end switch (st)
  } // end while
  *t = 0; // die restlichen zeichen koennten an dieser Stelle abgeschnitten
  // werden
  return;
}

char* TextFile::lastline(int n) {
  if (n<=0) n = 1;
  int b = bufferind;
// char *p = buffer+bufferind; 
  if (buffer[bufferind]=='\n') {
    buffer[(b+1)%buffersize] = 0;
    b = (b-1)%buffersize;
  }
  else {
    buffer[(b+1)%buffersize] = '\n';
    buffer[(b+2)%buffersize] = 0;
  }
  b += buffersize;
  char c;
  int i;
  for (i=0; i<buffersize; ++i) {
    c = buffer[(b-i)%buffersize];
    if (c=='\n') n--;
    if (n==0 || c=='\0') break;
  } 
  return buffer+((b+1-i)%buffersize);
}
      
int SimulObjectNode::ObjectType(char *token, SimulObjectType *t) {
  *t = _UNDEF;
  if (strcasecmp(token, "simul")==0) {
    *t = _SIMUL;
    return 1;
  }
  for (SimulObjectNode* n = this; n; n = n->parent) {
    if (n->creators.N()) {
      for (Pointer<SimulCreator> sc = n->creators; sc; ++sc) {
	*t = sc->type(token);
	if (*t) return 1;
      }
    }
  }
  return 0;
}
 
//  else if (strcasecmp(token, "attraction")==0) *t = _ATTRACTION;
//   else if (strcasecmp(token, "floor")==0) *t = _FLOOR;
//   else if (strcasecmp(token, "network")==0) *t = _NETWORK;
//   else if (strcasecmp(token, "environment")==0) *t = _ENVIRONMENT;
//   else if (strcasecmp(token, "edge")==0) *t = _EDGE;
//   else if (strcasecmp(token, "vertex")==0) *t = _VERTEX;
//   else if (strcasecmp(token, "display")==0) *t = _SCENERY_WINDOW;
//   else if (strcasecmp(token, "videodisplay")==0) *t = _VIDEO_WINDOW;
//   else if (strcasecmp(token, "replay")==0) *t = _REPLAY_WINDOW;
//   else if (strcasecmp(token, "netdisplay")==0) *t = _NET_WINDOW;
//   else if (strcasecmp(token, "plot")==0) *t = _SCENERY_PLOT;
//   else if (strcasecmp(token, "conturplot")==0) *t = _CONTUR_PLOT;
//   else if (strcasecmp(token, "movie")==0) *t = _MOVIE;
 
//   else *t = _UNDEF;
//   if (*t!=_UNDEF) {
//     test(printf("new object found \'%s\' type %d\n", token, *t));
//     return 1;
//   }
//   else return 0;
// }

SimulObjectNode* SimulObjectNode::reference(SimulObjectType tp,	char *c) {
  SimulObjectNode *nod=0;
  if (*c!='\0') {
    for (nod=tree; nod; nod=nod->next) {
      if (strcasecmp(c, nod->id)==0) {
	return  (nod && nod->obj && nod->type==tp) ? nod: NULL;
      }
    }
  }
  nod = new SimulObjectNode(this, tp, c);
  return (nod && nod->obj) ? nod : NULL;
}
	  
SimulObjectNode* SimulObjectNode::reference(char *c) {
  SimulObjectNode *par, *nod=0;
  if (c && *c!='\0') {
    test(printf("reference for `%s\': ", c));
    for (par=this; par; par=par->parent) {
      for (nod=par->tree; nod; nod=nod->next) {
	test(printf("%s ", nod->id));
	if (strcasecmp(c, nod->id)==0) {
	  test (printf("found!\n"));
	  return (nod && nod->obj) ? nod : NULL;
	}
      }
    }
  }
  test  (printf("failed!\n"));
  return NULL;
}


SimulObject* SimulObjectNode::CreateObject(SimulObjectType y) {
  SimulObject* obj = NULL;
  if (y==_SIMUL) {
    obj = new Simul;
    return obj;
  }
  for (SimulObjectNode* n = this; n; n = n->parent) {
    if (n->creators.N()) {
      for (Pointer<SimulCreator> sc = n->creators; sc; ++sc) {
	obj = sc->create(y);
	if (obj) return obj;
      }
    }
  }
  return NULL;
}

//   switch (y) {
//   case _UNDEF:
//   default: 
//     printf("unknown object-type.\n");
//     obj = NULL;
//     break;
// #ifdef _STREET_HH_
//     //  case _ENVIRONMENT:
//     //obj = new Environment(parobj);
//     //break;
//   case _STREET:
//     obj = new Street;
//     break;
//   case _CORRIDOR:
//     obj = new Corridor;
//     break;
//   case _POLYGON:
//     obj = new Polygon;
//     break;
//   case _CIRCLE:
//     obj = new Circle;
//     break;
//   case _DECORATION:
//     obj = new Decoration;
//     break;
//   case _GATE:
//     obj = new Gate;
//     break;
//   case _MULTIPLEGATE:
//     obj = new MultipleGate;
//     break;
//   case _DOOR:
//     obj = new Door;
//     break;
// #endif
// #ifdef _ATTRACTION_HH_
//   case _ATTRACTION:
//     obj = new Attraction;
//     break;
// #endif
// #ifdef _GROUND_HH_
//   case _GROUND:
//     obj = new Ground;
//     break;
//   case _MARKER:
//     obj = new Marker;
//     break;
// #endif
// #ifdef _ROUTE_HH_
//   case _VERTEX:
//     obj = new Vertex;
//     break;
//   case _EDGE:
//     obj = new Edge;
//     break;
//   case _NETWORK:
//     obj = new Network;
//     break;
// #endif
//   case _PEDESTRIAN:
//     obj = new Pedestrian;
//     break;
//   case _EVALPEDEST:
//     obj = new EvalPed;
//     break;
//     /* case _DATAPEDEST:
//        obj = new DataPedest;
//        break; */ 
//   case _FLOOR:
//     obj = new Floor;
//     break;
// #ifdef _MOVIE_HH_
//   case _MOVIE:
//     obj = new Movie();
//     break;
// #endif
//   case _SCENERY_PLOT: // UNIRAS
//     obj = newSceneryPlot();
//     break;
//   case _CONTUR_PLOT: // UNIRAS
//     obj = newConturPlot();
//     break;
//   case _SCENERY_WINDOW: // MOTIF
//     obj = newSzeneFenster(); // ACHTUNG! Keine "new SzeneFenster()"
//     break;
//   case _VIDEO_WINDOW: // MOTIF
//     obj = newVideoFenster(); // ACHTUNG! Keine "new SzeneFenster()"
//     break;
//   case _REPLAY_WINDOW: // MOTIF
//     obj = newReplayFenster(); // ACHTUNG! Keine "new SzeneFenster()"
//     break;
//   } // end switch
//   return obj;
// }

SimulObjectNode::SimulObjectNode(SimulObjectNode *par,SimulObjectType y,
				 char *c) {
  SimulObject *parobj = par? par->obj : NULL;
  obj = 0;
  obj = CreateObject(y);
  if (obj) {
    id  = new char[strlen(c)+1]; 
    if (*c!='\0') strcpy(id, c);
    else  *id = '\0';
    type = y;
    defined = 0;
    if (par) {
      parent = par;
      obj->parent = parent->obj;
      obj->parameter->parent = obj->parent->parameter;
      if (*id!='\0') {
	next = par->tree; // element in liste einhaengen
	par->tree = this;
      }
      else {
	next = 0;
      }
      tree = 0;
    }
    else { 
      parent = 0;
      obj->parent = 0;
      obj->parameter->parent = 0;
      tree = 0;
      next = 0;
    }
    test(printf("SimulObjectNode::new %d\n", type));
  } // endif object
  else Error("cannot create object.");
}

void SimulObjectNode::parse(TextFile *fp) {
  enum stat {S, Z, E1, E2, E3, E4, D1, D2, D3, D4, L, M1, M2, I1, F} st = S;
  char token[TOKENLENGTH+1];
  char identifier[TOKENLENGTH+1];
  char fehler[80];
  SimulObjectType tp;
  SimulObjectNode *pn, *par;
  SimulObject *cl;
  ParameterValue *p, pv;
  double *values = new double[32];
  Vector *vectors = new Vector[32];
  int nvalues = 0, nvectors = 0;
  int multi, nident, nmulti, npar;
  const int True = 1, False = 0;

  while(st!=Z && st!=F) {
    fp->token(token);
    test(printf("status: %d ->%s<-\n", st, token));
    switch (st) {
    case S: // Start
      multi = 0; nident = nmulti = 0;
      *identifier = '\0';
      par = this; npar = 0;
      tp = _UNDEF;
      //if (*token==(char)EOF) { // 
      if (fp->eof()) {
	st = Z; 
      }
      else if (*token==';' || *token==',') {
	st = S;
      }
      else if (*token=='}') {
	st = Z; 
      }
      // epression ?
      else if (index("-+.$1234567890()\'", *token)) {
	st = L;
	pv.scan(token);
	switch (pv.type) {
	case _real:
	  if (nvalues<32) {
	    values[nvalues++] = pv.value.d;
	  }
	  else {
	    Warning("Value capacity exceeded.");
	    st = F;
	  }
	  break;	  
	case _vec:
	  if (nvectors<32) {
	    vectors[nvectors++] = *pv.value.v;
	  }
	  else {
	    Warning("Vector capacity exceeded.");
	    st = F;
	  }
	  break;
	default: 
	  sprintf(fehler, "Value `%s\' must be real or real-vector.", token);
	  Error(fehler);
	  st = F;
	  break;  
	}
      }
      // class ?
      else if (ObjectType(token, &tp)) {
	st = E1;
      }
      // identifier ?
      else if (pn = reference(token), pn) {
	refref(obj, pn->obj);
	//printf("ref Type %d, N %d\n", obj->objecttype(), obj->refs.N());
	// pn->obj speichert die vorgesehenen Referenzen zu obl,
	// wenn alles fertig ist, werden die Referenzens in pn->init()
	// erzeugt.
      }
      // parameter ? ist eigentlich alles, was der Namenskonvention
      // entspricht. Abc123
      //else if (p = obj->parameter->resolve(token), p) {
      else { // if (p = obj->parameter->create(token), p) {
	strcpy(identifier, token);
	st = D1;
      }
//       else {
// 	sprintf(fehler,"Undefined parameter or identifier `%s\' used.",token);
// 	Error(fehler); 
// 	st = F;
//       }
      break;
    case E1: // lese identifier
      if (*token==';') { // nur erzeugen
	st = S;
      }
      else if (*token=='{') {
	if (nident<=0 && tp!=_UNDEF) {// anonymous obj.
	  pn = new SimulObjectNode(this, tp, ""); // erzeugen und dranhaengen
	  if (pn->obj) {
	    refref(obj, pn->obj);
	    // printf("ref Type %d, N %d\n", obj->objecttype(), obj->refs.N());
	  }
	  else {
	    delete pn;
	    pn = NULL;
	  }
	}
	if (pn && !pn->defined) {
	  pn->parse(fp);
	  pn->obj->init();
	  while (--multi>0) {
	    cl = pn->obj->clone(); // mach KEINE referenzen ...
	    cl->init();
	    refref(obj, cl);
	  }	    
	  st = S;
	}
	else { Error("already defined."); st = F; }
      }
      else if (*token=='[') {
	if (nmulti<=0) st = M1;
	else {
	  Error("multiple [].");
	  st = F;
	}
      }
      else if (*token==':') {
	if (npar<=0) st = I1;
	else {
	  Error("multiple inheritance.");
	  st = F;
	}
      }
      else if (nident<=0) {
	pn = reference(token);
	if (pn) {
	  if (pn->type==tp) {
	    refref(obj, pn->obj); 
	    // printf("ref Type %d, N %d\n", obj->objecttype(), obj->refs.N());
	    ++nident;
	    st = E1; // pn gefunden
	  }
	  else {Error("type conflict"); st = F;}
	}
	else if (tp!=_UNDEF) {
	  pn = new SimulObjectNode(this, tp, token);// erzeugen und dranhaengen
	  if (pn->obj) {
	    refref(obj, pn->obj); 
	    // printf("ref Type %d, N %d\n", obj->objecttype(), obj->refs.N());
	  }
	  else {
	    delete pn;
	    pn = NULL;
	  }
	  st = E1;
	  ++nident;
	}
	else {
	  Error("object not found and no class-type for new obj. specified");
	  st = F;
	}
      }
      else {
	Error("';' or declaration of new object required '{ ...'");
	st = F;
      } 
      break;
    case D1:
      if (*token=='=') {
	if (p = obj->parameter->create(identifier), p) {
	  st = D2;
	}
	else {
	  st = F;
	}
      } // end =
      else if (*token==';' || *token=='}') { // parameter ins array
	ParameterValue *p1 = obj->parameter->rresolve(identifier);
 	if (p1) {
	  if (*token==';') st = S;
	  else st = Z;
	  switch (p1->type) {
	  case _real:
	    if (nvalues<32) {
	      values[nvalues++] = p1->value.d;
	    }
	    else {
	      Warning("Value capacity exceeded.");
	      st = F;
	    }
	    break;	  
	  case _vec:
	    if (nvectors<32) {
	      vectors[nvectors++] = *(p1->value.v);
	    }
	    else {
	      Warning("Vector capacity exceeded.");
	      st = F;
	    }
	    break;
	  default: 
	    sprintf(fehler, "Invalid type of `%s\'.", identifier);
	    Error(fehler);
	    st = F;
	    break;  
	  }
	} // p1 gefunden 
	else {
	  sprintf(fehler, "Parmeter `%s\' undefined.", identifier);
	  Error(fehler);
	  st = F;
	}
      } // end ";}"
      else {
	Error("One of `=\', `;\' or `}\' required.");
	st = F;
      }
      break;
    case D2: // nach dem =-Zeichen
      if (p && p->scan(token)) {
	  st = L;
      }
      else if (p) {
 	ParameterValue *p1 = obj->parameter->rresolve(token);
 	if (p1) {
 	  *p = *p1; //p->value = p1->value; p->type = p1->type;
 	  st = L;
 	}
	else {
 	  sprintf(fehler, "invalid expression `%s\'", token);
 	  Error(fehler);
 	  st = F;
 	}
      }
      else {
	sprintf(fehler, "cannot create parameter before `%s\'", token);
	Error(fehler);
	st = F;
      }
      break;
    case L: 
      if (*token==';') {
	st = S;
      }
      else if (*token=='}') { // CLOSE Braket, values uebergeben
	st = Z;
      }
      else {
	Error("';' or '}' required");
	st = F;
      }
      break;
    case M1:
      pv.scan(token);
      if (pv.type==_int) {
	multi = pv;
	nmulti++;
	st = M2;
      }
      else {
	Error("integer value expected.");
	st = F;
      }
      break;
    case M2:
      if (*token==']') st = E1;
      else {
	Error("']' required");
	st = F;
      }
      break;
    case I1:
      if (par = reference(token), par) {
	npar++;
	pn->parent = par;
       	pn->obj->parameter->parent = par->obj->parameter;
	st = E1;
      }
      else {
	Error("can't inherit from undefined object");
	st = F;
      }
      break;
    default:
      Error("Undefined state"); return;
    } // end switch
  } // end while
  if (st==F) {
    int col = fp->column();
    sprintf(fehler, 
	    "PARSE:: errors ocurred in line %d col %d, parsing aborted\n",
	    fp->line()+1, col+1);
    Message(fehler);
    Message(fp->lastline());
    for (int i=0; i<col; ++i) Message("-");
    Message("^\n");
  }
  if (nvalues) {
    obj->parameter->a = new double[nvalues];
    obj->parameter->na = nvalues;
    for (int i=0; i<nvalues; i++) {
      obj->parameter->a[i] = values[i];
    }
  }
  if (nvectors) {
    obj->parameter->v = new Vector[nvectors];
    obj->parameter->nv = nvectors;
    for (int i=0; i<nvectors; i++) {
      *(obj->parameter->v+i) = vectors[i];
    }
  }
  // falls object noch nichts geerbt hat
  // if (obj->parent && !obj->parameter->parent) {
  //   obj->parameter->parent = obj->parent->parameter;
  //  }
  // else { obj->parameter->parent = 0;}  
}

// SIMUL

Simul::Simul() {
  // create parameters
  if (!parameter)
    parameter = new Parameter();
  movie_in = movie_out = -1;
}

Simul::~Simul() {
  if (movie_in>-1) close(movie_in);
  if (movie_out>-1) close(movie_out);
}

void Simul::init() {
  SimulObject::init();
  // Variablen festlegen
  get(parameter, "stop", &Tstop, 1e20);
  get(parameter, "dt", &Dt, 0.1);
  get(parameter, "display_rate", &display_rate, 1);
  get(parameter, "plot_rate", &plot_rate, 0);
  get(parameter, "record_rate", &record_rate, 0);
  get(parameter, "log_rate", &log_rate, 0);
  get(parameter, "display_wait", &display_wait, 0.0);
  int fpm;
  get(parameter, "display_fpm", &fpm, 0);
  display_frametime = fpm>0 ? 60.0/fpm : 0.0;
  display_t0.time = 0;
  display_t0.millitm = 0;
  get(parameter, "plot_wait", &plot_wait, 0.0);
  get(parameter, "record_wait", &record_wait, 0.0);
  get(parameter, "filename", &moviename, "movie.data");
  T = 0.0;
//   char timetxt[7];
//   sprintf(timetxt, "%06.0lf", T*10.0);
//   setenv("XU_TIME", timetxt, 1);
  char timetxt[80];
  sprintf(timetxt, "XU_TIME=%06.0lf", T*10.0);
  putenv(timetxt);
  counter = 0;
}

int Simul::ref(SimulObject *obj, SimulObjectType type) {
  int good = 0;
  switch (type) {
  case _FLOOR:
    floors.append(obj);
    good = 1;
    break;
  case _MOVIE:
    movies.append(obj);
    good = 1;
    break;
  case _WINDOW: // und alle Ableitungen
    displays.append(obj);
    good = 1;
    break;
  case _PLOT: // und alle Ableitungen
    plots.append(obj);
    good = 1;
    break;
  case _WALKER:
    walkers.append(obj);
    good = 1;
    break;
  case _GROUND: 
    grounds.append(obj);
    good = 1;
    break;
  default:
    good = 0;
    break;
  }
  if (good) return 1;
  else return SimulObject::ref(obj, type);
}


extern "C" off_t tell(int);

Actor::Actor() {
  pops = 0;
}

Actor::~Actor() {
}


void Actor::init() {
  SimulObject::init();
  get(parameter, "popsize", &pops, 1);
  if (pops<1) pops = 1;
}

void WithLogfile::init() {
  SimulObject::init();
  char *name;
  get(parameter, "logfile", &name, "");
  if (!logfp && *name) logfp = fopen(name, "w");
}
  

