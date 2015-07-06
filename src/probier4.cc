#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */
#include <iostream.h>
#include "vector.hh"
#include "list.hh"
#include "simultypes.hh"

const int SimulRecordHeaderMaxChannels = 16;
struct SimulRecordHeader {
  unsigned int T : 32;
  unsigned int N : 32;
  SimulObjectType type[SimulRecordHeaderMaxChannels];
  unsigned int Number[SimulRecordHeaderMaxChannels];
  int write(int s) {return ::write(s, this, sizeof(SimulRecordHeader));}
  int read(int s) {
    T = 0; N =0;
    return ::read(s, this, sizeof(SimulRecordHeader));
  }
};


enum WalkerRecordBlockType { WRBmisc, WRBdata, WRBdata2, WRBdata3 };
enum WlakerRecordBlockControl {WRB_START = 0, /* weitere ,*/ WRB_END = 31};
class WalkerRecordBlock {
public: 
  WalkerRecordBlockType type : 2;
  unsigned int continued : 1;
  union {
    struct {
      unsigned int control : 5; 
      unsigned int version : 32; // 4 bytes 
      float time;                // 4 bytes 
      unsigned int number;       // 4 bytes   
    } misc;
    struct {
      unsigned int clan : 5;
      float x, y;   // 8 bytes 
      float vx, vy; // 8 bytes Betrag der Geschwindigkeit
      float v0;     // 4 bytes Betrag der Wunschgeschw.
      unsigned int from_gate : 4; 
      unsigned int to_gate : 4;
    } data;
    struct {
      unsigned int dummy : 5;
      float a[5];    // 5*4 bytes Beschleunigung
      char c;
    } data2;
    struct {
      unsigned int dummy : 5;
      float a[5];    // 5*4 bytes Beschleunigung
      char c;
    } data3;
  };
  int write(int f) { return ::write(f, (char*)this, sizeof *this);} 
  int read(int f) { return ::read(f, (char*)this, sizeof *this);} 
};
      
class WalkerPop: public ListElement {
private: 
  struct WBlock {
    Vector pos;
    Vector vel;
    double v0;
    int clan;
  } *W;
  int WN;
  int Nmemo;
public:
  WalkerPop() {W = 0; WN = 0; Nmemo = 0;}
  ~WalkerPop();
  int read(int);
};


int WalkerPop::read(int movie) {
  printf("WalkerPop::read %d\n", movie);
  WalkerRecordBlock wrb;
  int res;
  int i = 0;
  do {
    res = wrb.read(movie);
//    printf("type %d\n", wrb.type);
    switch (wrb.type) {
    case WRBmisc:
      switch (wrb.misc.control) {
      case WRB_START:
	WN = wrb.misc.number;
	//T = wrb.misc.time;
	if (WN>Nmemo) {
	  Nmemo = WN;
	  if (W) delete W;
	  W = new WBlock[Nmemo];
	}
	printf("start: t=%f, n=%d, version %d\n", 
	       wrb.misc.time, wrb.misc.number, wrb.misc.version); 
	break;
      case WRB_END:
	WN = i;
	printf("blocks %d end.\n", WN);
	return res;
      }
      break;
    case WRBdata:
      W[i].pos.x = wrb.data.x;
      W[i].pos.y = wrb.data.y;
      //      printf(">> %d x=%f y=%f\n", i, wrb.data.x, wrb.data.y);
      W[i].vel.x = wrb.data.vx;
      W[i].vel.y = wrb.data.vy;
     
      W[i].v0 = wrb.data.v0;
      W[i].clan = wrb.data.clan;
      if (!wrb.continued) ++i;
      break;
    case WRBdata2:
      if (!wrb.continued) ++i;
      break;  
    case WRBdata3:
      if (!wrb.continued) ++i;
      break;        
    }
  } while (res>0);
  WN = i;
  return 0;
}


SimulRecordHeader head;  
int movie_in;
List subpops;

int Mread() {
//  Pointer<Ground> gro;
  Pointer<WalkerPop> pop;
  int res = 0;
  for (int i=0; i<head.N; ++i ) {
    printf("\n\nread channel %d, type %d\n", i, head.type[i]);
    switch (head.type[i]) {
    case _WALKER_POP:
      
      if (!subpops.N()) {
	cout << "new WalkerPop" << endl;
	WalkerPop *wp = new WalkerPop;
	//	wp->init();
	subpops.append(wp);
	res += wp->read(movie_in);
      }
      else {
	if (!pop) pop = subpops;
	else ++pop;
	res += pop->read(movie_in);
      }
      break;
    case _GROUND:
      break;
    }
  }
  return res;
}


class fix2 { // -1..+1
  int val : 16;
public:
  fix2() {val=0;}
  fix2(float x) {x *= 32767.0; val = int(x);}
  operator float() {return float(val)/32767.0;}
};

class fix4 { // -4e5..+4e5 
  int val : 32;
public:
  fix4() {val=0;}
  fix4(float x) {x*= 429496.7295; val = int(x);}
  operator float() {return float(val)/429496.7295;}
};


main () {
  double  x = 3.1415;
  fix4 a = x;
  fix4 b = 17.0/a;
  cout << a << "  " << b << "  " << a/b*b/a << endl;
}


