#ifndef _LIST_H_
#define _LIST_H_

class ListNode {
private:
  ListNode *next;
public:
  ListNode& insert(ListNode n) {n.next = this; return n;}
  ListNode* insert(ListNode *p) {p->next = this; return *p;}
  ListNode& append(ListNode n) {n.next = next; next = &n; return *this;}
  ListNode& append(ListNode *p) {p->next = next; next = p; return *this;}
  ListNode& remove() {return *this;}
  ListNode& operator++ () {return *next;}; 
  
  /*   ListNode& insert(ListNode n);
       ListNode* insert(ListNode *p);
       ListNode& append(ListNode n);
       ListNode& append(ListNode *p); 
       ListNode& remove();
       ListNode& operator++ ();  */
  
};

class ListElement {
friend class List; 
public:
  /* virtual ListElement () {}
     virtual ~ListElement () {} */
};

class List {
  struct Node {
    struct Node *succ, *prec;
    ListElement *cont;
    int refcount;
  } *top, *bottom;
  int number;
public:
  List () {top = bottom = 0; number=0;}
  List (ListElement& elem) {
    top = bottom = new Node;
    top->succ = top->prec = 0;
    top->cont = &elem;
    number = 1;
  }
  struct Node* operator() () const {return top;} 
  List& insert(ListElement& elem);
  List& append(ListElement& elem);
  List& remove(ListElement& elem);
  
  List& insert(ListElement *pelem);
  List& append(ListElement *pelem);
  List& remove(ListElement *pelem);
  
  List& insert(Node *n);
  List& append(Node *n);
  List& remove(Node *n);

  List& remove();
};

class RankingList: public List {
  struct Node {
    struct Node *succ, *prec;
    ListElement *cont;
    double rank;
    int refcount;
  } *top, *bottom;
public:
  List& sort();
};

template <class ListType>
class Pointer {
  List *list;
  List::Node *n;
public:
  Pointer () {list=0; n=0;}
  Pointer (List& l) {list=&l; n=l();}
  Pointer& operator= (List& l) {list=&l; n=l();} // reference copy
  Pointer& operator= (Pointer& point) {list=point.list; n=point.n;}
  Pointer& operator++ () {n=n->succ; return *this;}
  Pointer& operator-- () {n=n->prec; return *this;}
  ListType* operator-> () {return n? (ListType*)n->cont: 0;}
  ListType* operator() () {return n? (ListType*)n->cont: 0;}
  ListType& operator* () { return (ListType&)*(n->cont);}
  int valid() {return (int)n == 0 ? 0: 1;}
  // type conversion
  operator bool() {return (int)n == 0 ? 0: 1;}
  operator int() {return (int)n;}
  operator ListType*() {return n? (ListType*)n->cont: 0;}
//  operator ListElement*() {return (ListElement*)n->cont;}
  //int operator!= (int tag) {return (int)n != tag;}
  //int operator== (int tag) {return (int)n == tag;} 
  //List::Node* operator () () {return n;}
  //List::Node* operator List::Node*() {return n;} 
  ListType* operator[] (long int);
};
#endif
/* Please assert no stuff after this line */





