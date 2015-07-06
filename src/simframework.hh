#ifndef _SIMFRAMEWORK_HH_
#define _SIMFRAMEWORK_HH_

#ifdef NEED_STL_HEADERS
#warning Using 3rd party STL header!
#include "STL/vector"
#include "STL/string"
#else
#include <vector>
#include <string>
#endif

//using namespace std;


class Object {
protected: 
  unsigned int type;
  std::string typeName;
public:
  Object();
  virtual void init() = 0;
  virtual populationSize() = 0;
};

class SF_objlist_element { 
public: 
  Object* obj;
  std::string className;
  SF_objlist_element(Object* o, std::string n) {
    obj = o; className = n;
  }
};

extern SF_objlist_element* SF_objects[];
extern size_t SF_objects_size;

//extern std::vector<SF_objlist_element*> SF_objects;
int SF_initialize(int *argc, char ***argv);

/* Macros to create objects with parameters 

   Syntax:

   newObject(Class_of_objects, Name_of_this_class)
        {
	    inherit(Name_of_other_class);
        }


   newObjects(Class_of_objects, Name_of_this_class, Number_of_objects)

   newObject(Pedestrian, groupA) {
   inherit(groupB);
   v0 = { 0.0, 0.3};
   x0 = rect({0.0, 1.0}, {10.0, 11.0});
   loc = $(thirdFloor).loc;
   peers = $(".groupA-peers");
   }

*/
#define newObject(DTYPE, NAME) class Class_##NAME : public BaseClass_##DTYPE { public: Class_##NAME(); void init(); size_t populationSize() { return 1;} } NAME[1]; Class_##NAME::Class_##NAME() { std::cout << "create '" << #NAME << "'" << std::endl;  SF_objects[SF_objects_size++] = new SF_objlist_element(this, #NAME); } void Class_##NAME::init() 

#define newObjects(DTYPE, NAME, NUM) class Class_##NAME : public BaseClass_##DTYPE { public: Class_##NAME(); void init(); size_t populationSize() { return NUM;} } NAME[1]; Class_##NAME::Class_##NAME() { std::cout << "create '" << #NAME << "'" << std::endl;  SF_objects[SF_objects_size++] = new SF_objlist_element(this, #NAME); } void Class_##NAME::init() 



#define beginClass(NAME) class BaseClass_##NAME : public Object { public: BaseClass_##NAME() { std::cout << "create '" << #NAME << "'" << std::endl;  SF_objects[SF_objects_size++] = new SF_objlist_element(this, #NAME); }
#define endClass(NAME) } NAME[1];

/* use like
   beginClass(Pedestrian)
       public:
            double a;
   endClass(Pedestrian)
*/

#define $(ID) reference(#ID)
#define inherit(CLASS) Class_##CLASS::init()

#endif
