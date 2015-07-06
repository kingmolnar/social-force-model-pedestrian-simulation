#ifndef _HASH_HH_
#define _HASH_HH_

class HASH {
private:
  struct HASH_TABLE_STRUCT {
    char *name;
    void *id;
  } *table; // hash_table[hash_table_limit + 1];
  unsigned int N;
  int function (char *string);
public:
  HASH();
  HASH(int);
  ~HASH();
  int regist(char*, void*);
  int lookup(char*, void**);
  int lookup(char *t, int *i)
    {void *p; int s = lookup(t,&p); *i = (int)p; return s;}
};

template<class DATATYPE>
class Hash {
  HASH* h;
  union {
    void *P;
    DATATYPE *Q;
  };
public:
  Hash() {h = new HASH;}
  ~Hash() {delete h;}
  int regist(char *T, DATATYPE *D) {return h->regist(T, (void*)D);}
  int lookup(char *T, DATATYPE **D) {
    int status = h->lookup(T, &P);
    *D = Q;
    return status;
  }
  DATATYPE *lookup(char *T) {
    if (h->lookup(T, &P)) return Q;
    else return 0;
  }
};

#endif

