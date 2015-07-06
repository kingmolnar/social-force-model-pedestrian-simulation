#include <stdio.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */
#include "vector.hh"



double a[] = { 1.0, 2.0, 2.3, 4.5, 2.8, 1.9, 0.8, 3.7, 5.0 };
int n = sizeof(a) / sizeof(double); 

class XX {
public:
  const virtual char* identifier() { return "XX";}
  XX() { cout << "create XX\n";}
  ~XX() { cout << "destroy XX\n";}
  int number() {return 17;}
  double a;
};

class YY {
public:
  char* identifier() { return "YY";}
};

static XX* x = NULL;

template <class T> class Status {
private:
  T future;
  T now;
public:
  Status() {};
  Status(const T& x) {future = now = x;}
  T operator= (const T& x) {return future = x;}
  //  const operator T&() {return now;}
  operator T() {return now;}
  //  void update() {bcopy(&future, &now, sizeof(T));}
  void update() {now = future;}
};



main () {
  Status<int> a =  x->number();
  Status<int>* b;
  b = new Status<int>[17];
  b[4] = 3;
  cout <<  a << endl;
  a = 12;
  cout << "a = 12" << endl;
  cout << a << endl;
  cout << a*b[4] << endl;
  a.update();
  cout <<  a << endl;
  cout << b[4] << endl;
}














