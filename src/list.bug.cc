//#include <iostream.h>
//#include <stdio.h>

#include "list.hh" 

/* ListNode& ListNode::insert(ListNode n) {
   n.next = this; this = &n; return *this;
   }
   ListNode* ListNode::insert(ListNode *p) {
   p->next = this; this = p; return *this;
   }
   ListNode& ListNode::append(ListNode n) {
   n.next = next; next = &n; return *this;
   }
   ListNode& ListNode::append(ListNode *p) {
   p->next = next; next = p; return *this;
   }
   ListNode& ListNode::remove() {
   this = next; return *this;
   }
   ListNode& ListNode::operator++ () {return *next;}
   
   */

List& List::insert(ListElement& elem) {
  struct Node *next = new Node;
  next->cont = &elem;
  next->refcount++;
  number++;
  if (top==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->succ = top;
    next->prec = 0;
    top->prec = next;
    top = next;
  }
  return *this;
}

List& List::insert(ListElement *pelem) {
  struct Node *next = new Node;
  next->cont = pelem;
  next->refcount++;
  number++;
  if (top==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->succ = top;
    next->prec = 0;
    top->prec = next;
    top = next;
  }
  return *this;
}

List& List::insert(Node *next) {
  next->refcount++;  
  number++;
  if (top==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->succ = top;
    next->prec = 0;
    top->prec = next;
    top = next;
  }
  return *this;
}

List& List::append(ListElement& elem) {
  struct Node *next = new Node;
  next->cont = &elem;
  next->refcount++;
  number++;
  if (bottom==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->prec = bottom;
    next->succ = 0;
    bottom->succ = next;
    bottom = next;
  }
  return *this;
}

List& List::append(ListElement *pelem) {
  struct Node *next = new Node;
  next->cont = pelem;
  next->refcount++;
  number++;
  if (bottom==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->prec = bottom;
    next->succ = 0;
    bottom->succ = next;
    bottom = next;
  }
  return *this;
}

List& List::append(Node *next) {
  next->refcount++;
  number++;
  if (bottom==0) {
    top = bottom = next;
    next->prec = next->succ = 0;
  }
  else {
    next->prec = bottom;
    next->succ = 0;
    bottom->succ = next;
    bottom = next;
  }
  return *this;
}

List& List::remove(ListElement& elem)
{
  remove(&elem);
}

List& List::remove(ListElement* pelem)
{
  Node *n = top;
  for (n=top; n; n=n->succ) {
    if (n->cont==pelem) {
      remove(n);
      break;
    }
  }
  if (!n) {
    // Fehler: Element nicht in der Liste gefunden!
  }
  return *this;
}

List& List::remove(Node *next) {
  next->refcount--;
  number--;
  if (next->prec) next->prec->succ = next->succ;
  else top = next->succ;
  if (next->succ) next->succ->prec = next->prec;
  else bottom = next->prec;
  delete next;
  return *this;
}

List& List::remove() {
  Node *next = top;
  next->refcount--;
  number--;
  if (next->succ) next->succ->prec = 0;
  else bottom = 0;
  top = next->succ;
  delete next;
  return *this;
}


template<class ListType>
ListType* Pointer<ListType>::operator[] (long int id) {
  for (Pointer<ListType> p=list; p; ++p) {
    if (p->n->cont->ListElementId == id) {
      n = p->n;
      return (ListType*)(p->n->cont);
    }
  }
  n = 0;
  return 0; // (ListType*)(list()->cont); 
}
			  



