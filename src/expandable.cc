// ARRAY CLASS evtl. auch mal extern

#define ArrayType int
class Array {
private: 
  int Nblocksize;
  struct ArrayBlock { 
    ArrayType *a;
    int n; 
    ArrayBlock *next;
    ArrayBock() {next = 0; a = new ArrayType[Nblocksize]; Nblocks++;}
    ~ArrayBlock() {if (a) delete a; Nblocks--;}
  } *top, *bottom, *iblock;
  
  void del(ArrayBlock *bl) {
    if (bl->next) del(bl->next);
    else delete(bl);
  }
  int Nelements;
  int Nblocks;
  int indexblock;
public: 
  ArrayType(int elements=0, size=128) {
    Nelements = elements;
    Nblocksize = size;
    int blocks = Nelements/Nblocksize;
    for (int i=0; i<blocks; ++i) {
      ArrayBlock *bl = new ArrayBlock;
      if (i<blocks-1) bl->n = Nblocksize;
      else bl->n = Nelements%Nblocksize;
      bl->next = 0;
      if (i==0) bottom = top = bl;
      else {
	bottom->next = bl;
	bottom = bottom->next;
      } 
    }
  } 
  ~ArrayType() {del(top);}
  int N() {return Nelements;}
  ArrayType append(ArrayType& e) {
    if (bottom->n<Nblocksize) bottom->a[n++] = e;
    else {
      bottom->next = new ArrayBlock;
      bottom = bottom->next;
      bottom->n = 1;
      bottom->a[0] = e;
    }
    return e;
  }
  ArrayType& operator[](int x) {
    assert(x>=Nelements); 
    if (x/Nblocksize!=indexblock) {
      indexblock = x/Nblocks;
      iblock = anker;
      for (int nn=0; nn<indexblock; nn++) iblock = iblock->next;
    }
    return iblock->a[x%Nblocksize]; 
  } 
};
