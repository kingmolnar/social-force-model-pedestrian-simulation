#include <iostream>
#include "simframework.hh"


SF_objlist_element* SF_objects[256];
size_t SF_objects_size = 0;
//std::vector<SF_objlist_element*> SF_objects;

int SF_initialize(int *argc, char ***argv) {
  size_t n = SF_objects_size;
  std::cout << "Number of objects " << n << std::endl;
  for (int k=0; k<n; k++) {
    std::cout << k << ": " << SF_objects[k]->className << "   " << SF_objects[k]->obj << std::endl;
  }
}

Object::Object() {
  std::cout << "create Object" << std::endl;
  //  SF_objects.push_back(new SF_objlist_element(this, "Object")); 
  SF_objects[SF_objects_size++] = new SF_objlist_element(this, "Object");
}
