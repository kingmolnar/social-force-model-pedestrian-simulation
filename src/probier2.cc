#include <iostream.h>

// Grosses Feld


class A {
public: 
  virtual int schreib(int i) {cout << "class A" << endl; return 0;}
};

//A (*)(int) Feld[10];
int FeldIndex = 0;
#define AA
#ifndef AA
#error A noch nicht definiert
#endif

class B: public A {
public: 
  schreib(int i) {
    if (i==0) cout << "class B" << endl;
    else A::schreib(--i);
  }
};

class C: public B {
public: 
  schreib(int i) {
    if (i==0) cout << "class C" << endl;
    else B::schreib(--i);
  }
};

class D: public C {
};  

class E: public D {
public: 
  schreib(int i) {
    if (i==0) cout << "class E" << endl;
    else C::schreib(--i);
  }
};


main () {

  int N = 4000*2000;
  double *Feld = new double[N];
  cout << "Feldgroesse=" << sizeof(double)*N << endl;
  for (int i=0; i<N; ++i) Feld[i] = 7.0;
  A *a1, *a2, *a3;
  B *b;

  a1 = new A;
  cout << "A als A0: "; a1->schreib(0);
  b = new B;
  cout << "B als B0: "; b->schreib(0);
  a2 = b;
  cout << "B als A0: "; a2->schreib(0);
  cout << "B als A1: "; a2->schreib(1);
  E *e = new E;
  a3 = e;
  cout << "E als A0: "; a3->schreib(0);
  cout << "E als A1: "; a3->schreib(1);
  cout << "E als A2: "; a3->schreib(2);
  cout << "E als A3: "; a3->schreib(3);
  
}
