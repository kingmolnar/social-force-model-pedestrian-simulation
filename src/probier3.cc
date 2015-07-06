#include <iostream.h>

double x = 1;

class A {
public:
  struct block {
    double x;
  } m;
};

class B: public A {
public:
  struct block: public A::block {
    double y;
  } m;
  B(double c, double d) {m.x = c; m.y = d;}
  block *feld; 
};

main() {
  cout << x  << endl;
  int i = 0;
  B b(1.0, 2.0);
  cout << b.m.x << ' ' << b.m.y << endl; 
}
  
