//#include <iostream.h>
//#include <stdio.h>

#include "list.hh" 

List& List::insert(ListElement& elem) {
  ListNode *next = new ListNode;
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
  ListNode *next = new ListNode;
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

List& List::insert(ListNode *next) {
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
  ListNode *next = new ListNode;
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
  ListNode *next = new ListNode;
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

List& List::append(ListNode *next) {
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
  ListNode *n = top;
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

List& List::remove(ListNode *next) {
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
  ListNode *next = top;
  next->refcount--;
  number--;
  if (next->succ) next->succ->prec = 0;
  else bottom = 0;
  top = next->succ;
  delete next;
  return *this;
}





