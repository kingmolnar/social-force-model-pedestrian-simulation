#include "stdio.h"
#include "stdlib.h"
#include "iostream.h"
#include "stdlib.h"
#include "list.hh"
#include "network.hh"
#include "netfenster.hh"

#include <X11/Xt/Intrinsic.h>

#ifndef NULL
#define NULL 0
#endif 

class Knoten: public VertexClass /* , public ListElement */ {
public:
  int id;
};

class Kante: public EdgeClass /* , public ListElement */  {
};




main (int argc, char* argv[]) {
  int found;
  int nv = 0, ne = 0;
  int nr, a, b;
  double x, y;
  
  List knoten;
  List kanten;

  XtToolkitInitialize();
  XtAppContext context = XtCreateApplicationContext();
  Display *display = XtOpenDisplay(context, NULL, argv[0], "example",
			  0, 0, &argc, argv);
  if (display == 0) {
    fprintf(stderr, "%s:  Can't open display\n", argv[0]);
    exit(1);
  }
  

  for (x=0.0; x<=10.0; x+=2.0) {
    for (y=0.0; y<=10.0; y+=2.0) {
      Knoten *kn = new Knoten;
      kn->id = nv;
      kn->P.x = x;
      kn->P.y = y;
      knoten.append(kn);
      ++nv;
    }
  }
  cout << "\nAnzahl Knoten: " << nv << endl;
  
  for (int i=0; i<35; ++i) {
    a = i+1;
    if ((a%6)>0) { // WRITE (*,'(2I12)') I, NACHB1;
      Kante *ka = new Kante;
      found = 0;
      for (Pointer<Knoten> pk=knoten; pk; ++pk) {
	if (pk->id==a || pk->id==i) {
	  ka->vertices.append(*pk);
	  ++found;
	}
	if (found>=2) break;
      }
      kanten.append(ka);
      ++ne;
    }
    
    b = i+6;
    if (b<36) { // WRITE (*,'(2I12)') I, NACHB2
      Kante *ka = new Kante;
      found = 0;
      for (Pointer<Knoten> pk=knoten; pk; ++pk) {
	if (pk->id==b || pk->id==i) {
	  ka->vertices.append(pk());
	  ++found;
	}
	if (found>=2) break;
      }
      kanten.append(ka);
      ++ne;
    }
  } // end for
  cout << "\nAnzahl Kanten: " << ne << endl;

  Network *netz = new Network(knoten, kanten, 0L);
  //netz->write();
  
  netz->warshall();
  netz->count();
  

  cout << "AUSGABE!" << endl;
  
  for (Pointer<Kante> pa=kanten; pa; ++pa) {
    cout << "Kante: ";
    for (Pointer<Knoten> pk=pa->vertices; pk; ++pk) {
      cout << pk->id << " ";
    }
    cout << "f=" << pa->frequency << endl;
  }
  NetFenster *f = new NetFenster(display, netz);
  XtAppMainLoop(context);
}










