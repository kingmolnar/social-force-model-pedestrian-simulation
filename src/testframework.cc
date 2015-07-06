#include <iostream>
#include <string>
#include "simframework.hh"

using namespace std;

beginClass(pedestrian)
protected:
double x;
double v;
public:
void init() {
  std::cout << "init 'pedestrian'"<< std::endl;
}


endClass(pedestrian)


newObject(pedestrian, groupA) {
  x = 1.0;
  v = 1.4;
}


int main(int argc, char** argv) {
  std::cout << "start main" << std::endl;
  SF_initialize(&argc, &argv);
  std::cout << "end main" << std::endl;
  return 0;
}
  

