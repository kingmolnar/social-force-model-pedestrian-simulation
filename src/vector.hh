#ifndef _vector_hh_
#define _vector_hh_
#include "math.h"

template <class VectType>
class VECTOR {
public:
  VectType x, y;
  int dim() { return 2; }
  VECTOR() {x = y = VectType(0.0);}
//  VECTOR(VECTOR a) {x = a.x; y = a.y;} 
  VECTOR(VectType xx, VectType yy) {x = xx; y = yy;}
  double length() {return sqrt(x*x+y*y);}
    
  VECTOR& operator= (const VECTOR& a) {x = a.x; y = a.y;}
  VECTOR& operator+= (const VECTOR& a) {x += a.x; y += a.y;}
  VECTOR& operator-= (const VECTOR& a) {x -= a.x; y -= a.y;}
  VECTOR& operator*= (const VectType s) {x *= s; y *= s;}
  VECTOR& operator/= (const VectType s) {x /= s; y /= s;}
  
  VECTOR operator+ (const VECTOR& a) {
    VECTOR v(x+a.x, y+a.y);
    return v;
  }

  VECTOR operator- (const VECTOR& a) {
    VECTOR v(x-a.x, y-a.y);
    return v;
  }

  VectType operator* (const VECTOR& a) {
    return x*a.x + y*a.y;
  }

  VECTOR operator* (VectType s) {
    VECTOR v(x*s, y*s);
    return v;
  }

  VECTOR operator/ (VectType s) {
    VECTOR v(x/s, y/s);
    return v;
  }

  VECTOR operator- () {
    return VECTOR(-x, -y);
  }

  VECTOR operator! () { // senkrechter Vektor
    return VECTOR(-y, x);
  }
  
  int operator== (const VECTOR& a) {
    return x==a.x && y==a.y;
  }
  
  int operator!= (const VECTOR& a) {
    return x!=a.x || y!=a.y;
  }
};

typedef VECTOR<int> IntVector;

class Vector {
public:
  double x, y;
  int dim() { return 2; }
  Vector() {x = y = double(0.0);}
  //  Vector(Vector a) {x = a.x; y = a.y;} 
  Vector(double xx, double yy) {x = xx; y = yy;}
  double length() {return sqrt(x*x+y*y);}  
  Vector& operator= (const Vector& a) {x = a.x; y = a.y;}
  Vector& operator+= (const Vector& a) {x += a.x; y += a.y;}
  Vector& operator-= (const Vector& a) {x -= a.x; y -= a.y;}
  Vector& operator*= (const double s) {x *= s; y *= s;}
  Vector& operator/= (const double s) {x /= s; y /= s;}
  
  Vector operator+ (const Vector& a) {
    Vector v(x+a.x, y+a.y);
    return v;
  }
  
  Vector operator- (const Vector& a) {
    Vector v(x-a.x, y-a.y);
    return v;
  }
  
  double operator* (const Vector& a) {
    return x*a.x + y*a.y;
  }
  
  Vector operator* (double s) {
    Vector v(x*s, y*s);
    return v;
  }
  
  Vector operator/ (double s) {
    Vector v(x/s, y/s);
    return v;
  }
  
  Vector operator- () {
    return Vector(-x, -y);
  }
  
  Vector operator! () { // senkrechter Vektor
    return Vector(-y, x);
  }
  
  int operator== (const Vector& a) {
    return x==a.x && y==a.y;
  }
  
  int operator!= (const Vector& a) {
    return x!=a.x || y!=a.y;
  }
};

Vector operator+(const Vector& a, const Vector& b) {
  return Vector(a.x+b.x, a.y+b.y);
}
Vector operator-(const Vector& a, const Vector& b) {
  return Vector(a.x-b.x, a.y-b.y);
}
double operator*(const Vector& a, const Vector& b) {
  return (a.x*b.x + a.y*b.y);
}
Vector operator*(const Vector& a, const double& s) {
  return Vector(a.x*s, a.y*s);
}
Vector operator/(const Vector& a, const double& s) {
  return Vector(a.x/s, a.y/s);
}


#endif


