#define NTEST
#include "test.h"
#include <assert.h>
#include "stdio.h"
#include "iostream.h"
#include "network.hh"

// #ifndef MOTIF
void VertexClass::draw(Fenster *f, Drawable drw) {
}
void EdgeClass::draw(Fenster *f, Drawable drw) {
}
// #endif

Net::Net() { 
  /*  test(printf("constructor Net..."));
      //  fill();
      T = 0;
      if (treshold>0.0 && !rs) rs = new RanSequence(0,N-1);
      test(printf("done\n")); */
} 


void Net::fill() {
  test(printf("Net::fill..."));
//  if (matrix) delete matrix;
  matrix = new NetMatrixElement[N*N];
  //Vert = new VertexPointer[N];
  int n = 0;

  // indexing
  test(printf("index %d\n", N));
  for (int nn=0; nn<N; ++nn) {
    Vert[nn]->index(nn);
  }

  int ne = 0, l;
  Index a, b, i, j;
  EdgePointer e;
  for (l=0; l<Ne; ++l) {
    e = Edg[l];
    for (Index a=0; a<e->Nvert; ++a) {
      i = e->vert[a]->index();
      for (Index b=0; b<e->Nvert; ++b) {
	j = e->vert[b]->index();
	if (i == j) {
	  M(i,j)->next = NULL; // is gut fuer die Abbruchbed. j();
	  M(i,j)->value = 0.0;
	}
	else {
	  M(i,j)->edge = e; // Edg[l];
	  M(i,j)->next = e->vert[b]; //Edg[l]->vert[j];
	  M(i,j)->value = (e->vert[b]->dest(e->vert[a]->P)).length();
	  //(Edg[l]->vert[j]->dest(Edg[l]->vert[i]->P)).length();
	  test(printf("edg %d #%d #%d length=%lf\n",
		      ne, 
		      e->vert[a]->index(),
		      e->vert[b]->index(), 
		      M(i,j)->value)
	       );
	  ++ne;
	}
      }
    }
  }
  //cout << "Net::fill ne=" << ne << endl;
  test(printf("done\n"));
}

int Net::warshall() { // Routen Suchen 
  Index i, j, k, ii, jj, kk;
  Index x[N];
  if (rs) {
    i = 0;
    for (rs->start(); rs->cont(); ++(*rs)) x[i++] = int(*rs);
  }
  else for(i=0; i<N; ++i) x[i] = i;
#ifndef NTEST 
  printf("ws indexing\n");
  for(i=0; i<N; ++i) printf("%d ", x[i]);
  printf("\n");
#endif
  int n=0;
  double val;
  /* der suchalgorithmus nach Warshall */
  for (j=0; j<N; ++j) { /* DO J=1,NP */
    jj = x[j];
    for (i=0; i<N; ++i) { /* DO I=1,NP */
      ii = x[i];
      if ( valid(ii,jj) ) {
	for (k=0; k<N; ++k) {
	  kk = x[k];
	  if ( valid(jj,kk) ) {
	    if ( length(ii,kk) > length(ii,jj) + length(jj,kk) ) {
	      link(ii, jj, kk);     //M(ii,kk)->link(M(ii,jj), M(jj,kk));
	      ++n;
	    }
	  }
	}
      }
    }
  }
  return n;
}


/* int Net::count() {
  int n = 0;
  Index i, j, k;
  for (i=0; i<N; ++i) {
    for (j=0; j<N; ++j) {
      if (M(i,j)->weight>0 && M(i,j)->valid()) { // gehe den weg
	for (k=i; k!=j; k=M(k,j)->next->index()) {
	  if (M(k,j)->edge) {
	    M(k,j)->edge->frequency += M(i,j)->weight;
	    ++n;
	  }}}}} return n;} */

int Net::count(int *delta, double *error) {
  int n = 0;
  int zuwachs = 0;
  double fehler = 0.0;
  if (error) *error = 0.0;
  Index list[N], i, l, ll, ii, kk;
  VertexClass *j;
  int zz;
  for (i=0; i<N; ++i) {
    for (l=0; l<N; ++l) {
      // zz =  Vert[i].weight * Vert[l].weight; // oder eine lastfunktion
      zz = M(i, l)->weight;
      kk = 0; 
      list[kk] = l;
      ii = i;
      ll = l;
      j = M(ii, ll)->next; 
      while (j) { 
	if (j->index() == ll) {
	  zuwachs += zz;
	  M(ii, j->index())->z += zz;
	  M(ii, j->index())->edge->frequency += zz;
	  fehler = double(zz)/double(M(ii, j->index())->edge->frequency);
	  n++;
	  ii = j->index();
	  ll = list[kk--]; 
        }
	else {
	  list[++kk] = ll;
          ll = j->index(); 
        }
	j = M(ii, ll)->next;
      }
    } 
  } 
  if (error) *error = fehler;
  if (delta) *delta = zuwachs;
  return n;
}

void Net::write() {
  cout << "Vertices\n";
  for (int i=0; i<N; ++i) {
    cout << Vert[i]->index() << ":  x=" << Vert[i]->P.x << 
      " y="<< Vert[i]->P.y<<endl;
  }

  cout << "Edges\n";
  for (int l=0;l<Ne; ++l) {
    cout << Edg[l]->frequency << ":  ";
    for (int j=0; j<Edg[l]->Nvert; ++j) {
      cout << " #" << Edg[l]->vert[j]->index() ;
    }
    cout << endl;
  }
  /* 
     cout << "\n\nMATRIX\n";
     for (int k=0; k<N*N; ++k) {
     if (matrix[k].edge) {
     cout << k << ": value=" << matrix[k].value <<
     " weight=" << matrix[k].weight;
     for (int j=0; j<matrix[k].edge->Nvert; ++j) {
     cout << " #" << matrix[k].edge->vert[j]->index() ;
     }
     cout << endl;
     }
     }
     */
}

/* 
Network::Network(List& ver, List& edg, unsigned long idum) {
  vertices = ver;
  edges = edg;
  fill(vertices, edges);
  T = 0;
  if (N>0 && idum>0) rs = new RanSequence(0,N-1);
  cout << T << '/' << N << endl;
  Pointer<VertexClass> i;
  int n =0;
  for (i=vertices; i; ++i) {
    if (n==0) Xmin = Xmax = i->P;
    else if (i->P.x < Xmin.x) Xmin.x = i->P.x;
    else if (i->P.y < Xmin.y) Xmin.y = i->P.y;
    else if (i->P.x > Xmax.x) Xmax.x = i->P.x;
    else if (i->P.y > Xmax.y) Xmax.y = i->P.y;
    ++n;
  }
}

Network::~Network() {
}

*/




