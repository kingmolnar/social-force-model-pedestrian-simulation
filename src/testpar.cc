//#include <stdio.h>
#include <iostream>
// Standard Template Library: use -I./STL compiler option
#ifdef NEED_STL_HEADERS
#warning Using 3rd party STL header!
#include "STL/vector"
#include "STL/string"
#else
#include <vector>
#include <string>
#endif


using namespace std;

#define eval( X ) do { printf("%s", #X); double xx = X; } while(0)

char* _list[100];

//#define obj(DTYPE, NAME) DTYPE NAME; void NAME##_init(void *__tt);  _list[ __COUNTER__ ] = #NAME; void NAME##_init(void *__tt) 

/* class Class_groupB : Pedestrian { Class_groupB(); } groupB[1];
Class_groupB::Class_groupB() { init(); } 
void Class_groupB::init() { x0 = {0.1, 0.3}; }
*/ 
#define Object(DTYPE, NAME) class Class_##NAME : DTYPE { Class_##DTYPE(); } NAME[1]; Class_##NAME::Class_##NAME() { init(); } void Class_##NAME::init() 

#define Objects(DTYPE, NAME, NUM) class Class_##NAME : DTYPE { Class_##DTYPE(); } NAME[NUM]; Class_##NAME::Class_##NAME() { init(); } void Class_##NAME::init() 


#define $(ID) reference(#ID)
#define inherit(CLASS) Class_##CLASS::init()
/* 
Object(Pedestrian, groupA) {
  inherit(groupB);
  v0 = { 0.0, 0.3};
  x0 = rect({0.0, 1.0}, {10.0, 11.0});
  loc = $(thirdFloor).loc;
  peers = $(".groupA-peers");
}

*/




class A {
  double x;
} aa;


class B : decltype(aa) {
} bb;



int main() {
  double a;
  //eval( a + 3.0 );
  vector<double> SS;

  SS.push_back(10.45);
  SS.push_back(20.007);
  SS.push_back(30.2339);
  
   cout << "Loop by index:" << endl;

   int ii;
   for(ii=0; ii < SS.size(); ii++)
   {
      cout << SS[ii] << endl;
   }

}
