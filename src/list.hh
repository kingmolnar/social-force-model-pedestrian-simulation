#ifndef _LIST_HH_
#define _LIST_HH_

class ListElement {
//friend class List; 
//public:
};

struct ListNode {
  ListNode() {succ = prec = 0; cont = 0; refcount = 0;}
  ListNode *succ, *prec;
  ListElement *cont;
    int refcount;
};

typedef ListElement* ListElementPointer;

class List {
  ListNode *top, *bottom;
  int number;
public:
  List () {top = bottom = 0; number=0;}
  List (ListElement& elem) {
    top = bottom = new ListNode;
    top->succ = top->prec = 0;
    top->cont = &elem;
    number = 1;
  }
  struct ListNode* operator() () const {return top;} 
  struct ListNode* last() {return bottom;} 
  List& insert(ListElement& elem);
  List& append(ListElement& elem);
  List& remove(ListElement& elem);
  
  List& insert(ListElement *pelem);
  List& append(ListElement *pelem);
  List& remove(ListElement *pelem);
  
  List& insert(ListNode *n);
  List& append(ListNode *n);
  List& remove(ListNode *n);

  List& remove();
  int N() {return number;}
};

template <class ListType> class Pointer {
private:
  ListNode *n;
  List *list;
public:
  Pointer() {list = 0; n = 0;}
  Pointer(List& l) {list=&l; n=l();} 
  Pointer& last(List& l) {list=&l; n = l.last();}
  Pointer& operator= (List& l) {list=&l; n=l();} // reference copy
  Pointer& operator= (Pointer& point) {list=point.list; n=point.n;}
  Pointer& operator++ () {if (n) n=n->succ; return *this;}
  Pointer& operator-- () {if (n) n=n->prec; return *this;}
  ListType* operator-> () {return n? (ListType*)n->cont: 0;}
  ListType* operator() () {return n? (ListType*)n->cont: 0;}
  ListType& operator* () { return (ListType&)*(n->cont);}
  int valid() {return (int)n == 0 ? 0: 1;}
  // type conversion
  operator bool() {return (int)n == 0 ? 0: 1;}
  operator int() {return (int)n;}
  operator ListType*() {return n? (ListType*)n->cont: 0;}
};

#endif
/* Please assert no stuff after this line */





