#include <iostream>
class Object {};


class BaseClass_pedestrian : Object { public: BaseClass_pedestrian() { std::cout << "create '" << "pedestrian" << "'" << std::endl; }
protected:
  double x;
  double v;
public:
  void init() {
    std::cout << "init 'pedestrian'"<< std::endl;
  }
  
  
} pedesrian[1];


class Class_pedestrian : BaseClass_groupA { public: Class_pedestrian(); } pedestrian[1]; Class_pedestrian::Class_pedestrian() { std::cout << "create '" << "pedestrian" << "'" << std::endl; } void Class_pedestrian::init() {
  x = 1.0;
  v = 1.4;
}
