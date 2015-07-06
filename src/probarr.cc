template<class STRUKTUR> class ARR {
private:
  STRUKTUR *a;
  int NN, MM;
public:
  ARR() {a = 0; NN = MM = 0;}
  //ARR(const ARR& x) {a = x.a; NN = x.NN; MM = x.MM;}
  ~ARR() {
    delete [] a;
  }
  dim(int n, int m) {
    if (a) delete [] a;
    NN = n; MM = m;
    a = new STRUKTUR[NN*MM];
  }
  STRUKTUR& operator() (int i) {return a[i];}
  STRUKTUR& operator() (int i, int j) {return a[i*MM+j];}
  //  const STRUKTUR* operator(STRUKTUR*)() {return a;}
  int N() {return NN;}
  int M() {return MM;}
};


main() {
  ARR<double> a;
  a.dim(10,10);
  for (int i=0; i<a.N(); ++i) {
    for (int j=0; j<a.M(); ++j) {
      a(i,j) = double(i*j);
    }
  }
}
    
