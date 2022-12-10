/*
    This file is part of "sphereEversion",
    a program by Michael McGuffin.
    The code in this file was almost entirely taken
    (with slight adaptations) from the source code of
    "evert", a program written by Nathaniel Thurston.
    evert's source code can be down loaded from
        http://www.geom.umn.edu/docs/outreach/oi/software.html
        http://www.geom.uiuc.edu/docs/outreach/oi/software.html

    Grateful acknowledgements go out to Nathaniel Thurston,
    Silvio Levy, and the Geometry Center (University of Minnesota)
    for making evert's source code freely available to the public.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "generateGeometry.h"

#ifdef _WIN32
#define M_PI 3.1415926535897932384626433832795
#endif

// ----------------------------------------

class TwoJet D(const class ThreeJet x, int index);
class TwoJet {
  public: /* this is a hack, but needed for now */
  double f;
  double fu, fv;
  double fuv;

  TwoJet() {}
  TwoJet(double d, double du, double dv)
   { f = d; fu = du; fv = dv; fuv = 0; }
  TwoJet(double d, double du, double dv, double duv)
   { f = d; fu = du; fv = dv; fuv = duv; }
#if 0
  operator double() { return f; }
#endif
  bool operator<(double d) { return f < d; }
  bool operator>(double d) { return f > d; }
  bool operator<=(double d) { return f <= d; }
  bool operator>=(double d) { return f >= d; }
  double df_du() { return fu; }
  double df_dv() { return fv; }
  double d2f_dudv() { return fuv; }
  void operator +=(TwoJet x)
   { f += x.f; fu += x.fu; fv += x.fv; fuv += x.fuv; }
  void operator +=(double d)
   { f += d; }
  void operator *=(TwoJet x)
   {
     fuv = f*x.fuv + fu*x.fv + fv*x.fu + fuv*x.f;
     fu = f*x.fu + fu*x.f;
     fv = f*x.fv + fv*x.f;
     f *= x.f;
   }
  void operator *=(double d)
   { f *= d; fu *= d; fv *= d; fuv *= d; }
  void operator %=(double d)
   { f = fmod(f, d); if (f < 0) f += d; }
  void operator ^=(double n)
   {
    if (f > 0) {
     double x0 = pow(f, n);
     double x1 = n * x0/f;
     double x2 = (n-1)*x1/f;
     fuv = x1*fuv + x2*fu*fv;
     fu = x1*fu;
     fv = x1*fv;
     f = x0;
    }
   }
  void Annihilate(int index)
   { if (index == 0) fu = 0;
     else if (index == 1) fv = 0;
     fuv = 0;
   }
  void TakeSin() {
   *this *= 2*M_PI;
   double s = sin(f), c = cos(f);
   f = s; fu = fu*c; fv = fv*c; fuv = c*fuv - s*fu*fv;
  }
  void TakeCos() {
   *this *= 2*M_PI;
   double s = cos(f), c = -sin(f);
   f = s; fu = fu*c; fv = fv*c; fuv = c*fuv - s*fu*fv;
  }

  friend TwoJet operator+(const TwoJet x, const TwoJet y);
  friend TwoJet operator*(const TwoJet x, const TwoJet y);
  friend TwoJet operator+(const TwoJet x, double d);
  friend TwoJet operator*(const TwoJet x, double d);
  friend TwoJet Sin(const TwoJet x);
  friend TwoJet Cos(const TwoJet x);
  friend TwoJet operator^(const TwoJet x, double n);
  friend TwoJet Annihilate(const TwoJet x, int index);
  friend TwoJet Interpolate(const TwoJet v1, const TwoJet v2, const TwoJet weight);
  friend class TwoJet D(const class ThreeJet x, int index);
  friend class ThreeJet;
};

// ----------------------------------------

TwoJet operator+(const TwoJet x, const TwoJet y) {
  return TwoJet(x.f+y.f, x.fu+y.fu, x.fv+y.fv, x.fuv + y.fuv);
}

TwoJet operator*(const TwoJet x, const TwoJet y) {
  return TwoJet(
    x.f*y.f,
    x.f*y.fu + x.fu*y.f,
    x.f*y.fv + x.fv*y.f,
    x.f*y.fuv + x.fu*y.fv + x.fv*y.fu + x.fuv*y.f
  );
}

TwoJet operator+(const TwoJet x, double d) {
  return TwoJet( x.f + d, x.fu, x.fv, x.fuv);
}

TwoJet operator*(const TwoJet x, double d) {
  return TwoJet( d*x.f, d*x.fu, d*x.fv, d*x.fuv);
}

TwoJet Sin(const TwoJet x) {
  TwoJet t = x*(2*M_PI);
  double s = sin(t.f);
  double c = cos(t.f);
  return TwoJet(s, c*t.fu, c*t.fv, c*t.fuv - s*t.fu*t.fv);
}

TwoJet Cos(const TwoJet x) {
  TwoJet t = x*(2*M_PI);
  double s = cos(t.f);
  double c = -sin(t.f);
  return TwoJet(s, c*t.fu, c*t.fv, c*t.fuv - s*t.fu*t.fv);
}

TwoJet operator^(const TwoJet x, double n) {
  double x0 = pow(x.f, n);
  double x1 = (x.f == 0) ? 0 : n * x0/x.f;
  double x2 = (x.f == 0) ? 0 : (n-1)*x1/x.f;
  return TwoJet(x0, x1*x.fu, x1*x.fv, x1*x.fuv + x2*x.fu*x.fv);
}

TwoJet Annihilate(const TwoJet x, int index) {
  return TwoJet(x.f, index == 1 ? x.fu : 0, index == 0 ? x.fv : 0, 0);
}

TwoJet Interpolate(const TwoJet v1, const TwoJet v2, const TwoJet weight) {
  return (v1) * ((weight) * (-1) + 1) + v2*weight;
}


// ----------------------------------------

class ThreeJet {
public: // hack
  double f;
private:
  double fu, fv;
  double fuu, fuv, fvv;
  double fuuv, fuvv;

  ThreeJet(double d, double du, double dv, double duu, double duv, double dvv,
   double duuv, double duvv)
   { f = d; fu = du; fv = dv; fuu = duu; fuv = duv; fvv = dvv;
     fuuv = duuv; fuvv = duvv; }
  public:
  ThreeJet() {}
  ThreeJet(double d, double du, double dv)
   { f = d; fu = du; fv = dv; fuu = fuv = fvv = fuuv = fuvv = 0;}
  operator TwoJet() { return TwoJet(f, fu, fv, fuv); }
#if 0
  operator double() { return f; }
#endif
  bool operator<(double d) { return f < d; }
  bool operator>(double d) { return f > d; }
  bool operator<=(double d) { return f <= d; }
  bool operator>=(double d) { return f >= d; }
  void operator %=(double d)
   { f = fmod(f, d); if (f < 0) f += d; }
  friend ThreeJet operator+(const ThreeJet x, const ThreeJet y);
  friend ThreeJet operator*(const ThreeJet x, const ThreeJet y);
  friend ThreeJet operator+(const ThreeJet x, double d);
  friend ThreeJet operator*(const ThreeJet x, double d);
  friend ThreeJet Sin(const ThreeJet x);
  friend ThreeJet Cos(const ThreeJet x);
  friend ThreeJet operator^(const ThreeJet x, double n);
  friend ThreeJet Annihilate(const ThreeJet x, int index);
  friend ThreeJet Interpolate(const ThreeJet v1, const ThreeJet v2, const ThreeJet weight);
  friend class TwoJet D(const class ThreeJet x, int index);
};

// ----------------------------------------

ThreeJet operator+(const ThreeJet x, const ThreeJet y) {
  ThreeJet result;
  result.f = x.f + y.f;
  result.fu = x.fu + y.fu;
  result.fv = x.fv + y.fv;
  result.fuu = x.fuu + y.fuu;
  result.fuv = x.fuv + y.fuv;
  result.fvv = x.fvv + y.fvv;
  result.fuuv = x.fuuv + y.fuuv;
  result.fuvv = x.fuvv + y.fuvv;
  return result;
}

ThreeJet operator*(const ThreeJet x, const ThreeJet y) {
  ThreeJet result;
  result.f = x.f*y.f;
  result.fu = x.f*y.fu + x.fu*y.f;
  result.fv = x.f*y.fv + x.fv*y.f;
  result.fuu = x.f*y.fuu + 2*x.fu*y.fu + x.fuu*y.f;
  result.fuv = x.f*y.fuv + x.fu*y.fv + x.fv*y.fu + x.fuv*y.f;
  result.fvv = x.f*y.fvv + 2*x.fv*y.fv + x.fvv*y.f;
  result.fuuv = x.f*y.fuuv + 2*x.fu*y.fuv + x.fv*y.fuu
           + 2*x.fuv*y.fu + x.fuu*y.fv + x.fuuv*y.f;
  result.fuvv = x.f*y.fuvv + 2*x.fv*y.fuv + x.fu*y.fvv
           + 2*x.fuv*y.fv + x.fvv*y.fu + x.fuvv*y.f;
  return result;
}

ThreeJet operator+(const ThreeJet x, double d) {
  ThreeJet result;
  result = x;
  result.f += d;
  return result;
}

ThreeJet operator*(const ThreeJet x, double d) {
  ThreeJet result;
  result.f = d*x.f;
  result.fu = d*x.fu;
  result.fv = d*x.fv;
  result.fuu = d*x.fuu;
  result.fuv = d*x.fuv;
  result.fvv = d*x.fvv;
  result.fuuv = d*x.fuuv;
  result.fuvv = d*x.fuvv;
  return result;
}

ThreeJet Sin(const ThreeJet x) {
  ThreeJet result;
  ThreeJet t = x*(2*M_PI);
  double s = sin(t.f);
  double c = cos(t.f);
  result.f = s;
  result.fu = c*t.fu;
  result.fv = c*t.fv;
  result.fuu = c*t.fuu - s*t.fu*t.fu;
  result.fuv = c*t.fuv - s*t.fu*t.fv;
  result.fvv = c*t.fvv - s*t.fv*t.fv;
  result.fuuv = c*t.fuuv - s*(2*t.fu*t.fuv + t.fv*t.fuu) - c*t.fu*t.fu*t.fv;
  result.fuvv = c*t.fuvv - s*(2*t.fv*t.fuv + t.fu*t.fvv) - c*t.fu*t.fv*t.fv;
  return result;
}

ThreeJet Cos(const ThreeJet x) {
  ThreeJet result;
  ThreeJet t = x*(2*M_PI);
  double s = cos(t.f);
  double c = -sin(t.f);
  result.f = s;
  result.fu = c*t.fu;
  result.fv = c*t.fv;
  result.fuu = c*t.fuu - s*t.fu*t.fu;
  result.fuv = c*t.fuv - s*t.fu*t.fv;
  result.fvv = c*t.fvv - s*t.fv*t.fv;
  result.fuuv = c*t.fuuv - s*(2*t.fu*t.fuv + t.fv*t.fuu) - c*t.fu*t.fu*t.fv;
  result.fuvv = c*t.fuvv - s*(2*t.fv*t.fuv + t.fu*t.fvv) - c*t.fu*t.fv*t.fv;
  return result;
}

ThreeJet operator^(const ThreeJet x, double n) {
  double x0 = pow(x.f, n);
  double x1 = (x.f == 0) ? 0 : n * x0/x.f;
  double x2 = (x.f == 0) ? 0 : (n-1) * x1/x.f;
  double x3 = (x.f == 0) ? 0 : (n-2) * x2/x.f;
  ThreeJet result;
  result.f = x0;
  result.fu = x1*x.fu;
  result.fv = x1*x.fv;
  result.fuu = x1*x.fuu + x2*x.fu*x.fu;
  result.fuv = x1*x.fuv + x2*x.fu*x.fv;
  result.fvv = x1*x.fvv + x2*x.fv*x.fv;
  result.fuuv = x1*x.fuuv + x2*(2*x.fu*x.fuv + x.fv*x.fuu) + x3*x.fu*x.fu*x.fv;
  result.fuvv = x1*x.fuvv + x2*(2*x.fv*x.fuv + x.fu*x.fvv) + x3*x.fu*x.fv*x.fv;
  return result;
}

TwoJet D(const ThreeJet x, int index) {
  TwoJet result;
  if (index == 0) {
    result.f = x.fu;
    result.fu = x.fuu;
    result.fv = x.fuv;
    result.fuv = x.fuuv;
  } else if (index == 1) {
    result.f = x.fv;
    result.fu = x.fuv;
    result.fv = x.fvv;
    result.fuv = x.fuvv;
  } else {
    result.f = result.fu = result.fv =
    result.fuv = 0;
  }
  return result;
}

ThreeJet Annihilate(const ThreeJet x, int index) {
  ThreeJet result = ThreeJet(x.f,0,0);
  if (index == 0) {
    result.fv = x.fv;
    result.fvv = x.fvv;
  } else if (index == 1) {
    result.fu = x.fu;
    result.fuu = x.fuu;
  }
  return result;
}

ThreeJet Interpolate(const ThreeJet v1, const ThreeJet v2, const ThreeJet weight) {
  return (v1) * ((weight) * (-1) + 1) + v2*weight;
}

// ----------------------------------------

struct TwoJetVec {
  TwoJet x;
  TwoJet y;
  TwoJet z;
  TwoJetVec() {}
  TwoJetVec(TwoJet a, TwoJet b, TwoJet c) { x = a; y = b; z = c; }
};

TwoJetVec operator+(TwoJetVec v, TwoJetVec w);
TwoJetVec operator*(TwoJetVec v, TwoJet  a);
TwoJetVec operator*(TwoJetVec v, double a);
TwoJetVec AnnihilateVec(TwoJetVec v, int index);
TwoJetVec Cross(TwoJetVec v, TwoJetVec w);
TwoJet Dot(TwoJetVec v, TwoJetVec w);
TwoJetVec Normalize(TwoJetVec v);
TwoJetVec RotateZ(TwoJetVec v, TwoJet angle);
TwoJetVec RotateY(TwoJetVec v, TwoJet angle);
TwoJetVec RotateX(TwoJetVec v, TwoJet angle);
TwoJetVec InterpolateVec(TwoJetVec v1, TwoJetVec v2, TwoJet weight);
TwoJet Length(TwoJetVec v);

// ----------------------------------------

TwoJetVec operator+(TwoJetVec v, TwoJetVec w) {
  TwoJetVec result;
  result.x = v.x + w.x;
  result.y = v.y + w.y;
  result.z = v.z + w.z;
  return result;
}

TwoJetVec operator*(TwoJetVec v, TwoJet  a) {
  TwoJetVec result;
  result.x = v.x*a;
  result.y = v.y*a;
  result.z = v.z*a;
  return result;
}

TwoJetVec operator*(TwoJetVec v, double a) {
  TwoJetVec result;
  result.x = v.x*a;
  result.y = v.y*a;
  result.z = v.z*a;
  return result;
}

TwoJetVec AnnihilateVec(TwoJetVec v, int index) {
  TwoJetVec result;
  result.x = Annihilate(v.x, index);
  result.y = Annihilate(v.y, index);
  result.z = Annihilate(v.z, index);
  return result;
}

TwoJetVec Cross(TwoJetVec v, TwoJetVec w) {
  TwoJetVec result;
  result.x = v.y*w.z + v.z*w.y*-1;
  result.y = v.z*w.x + v.x*w.z*-1;
  result.z = v.x*w.y + v.y*w.x*-1;
  return result;
}

TwoJet Dot(TwoJetVec v, TwoJetVec w) {
  return v.x*w.x + v.y*w.y + v.z*w.z;
}

TwoJetVec Normalize(TwoJetVec v) {
  TwoJet a;
  a = Dot(v,v);
  if (a > 0)
    a = a^-0.5;
  else
    a = TwoJet(0, 0, 0);
  return v*a;
}

TwoJetVec RotateZ(TwoJetVec v, TwoJet angle) {
  TwoJetVec result;
  TwoJet s,c;
  s = Sin (angle);
  c = Cos (angle);
  result.x =          v.x*c + v.y*s;
  result.y = v.x*s*-1 + v.y*c;
  result.z = v.z;
  return result;
}

TwoJetVec RotateY(TwoJetVec v, TwoJet angle) {
  TwoJetVec result;
  TwoJet s, c;
  s = Sin (angle);
  c = Cos (angle);
  result.x = v.x*c + v.z*s*-1;
  result.y = v.y;
  result.z = v.x*s + v.z*c    ;
  return result;
}

TwoJetVec RotateX(TwoJetVec v, TwoJet angle) {
  TwoJetVec result;
  TwoJet s,c;
  s = Sin (angle);
  c = Cos (angle);
  result.x = v.x;
  result.y = v.y*c + v.z*s;
  result.z = v.y*s*-1 + v.z*c;
  return result;
}

TwoJetVec InterpolateVec(TwoJetVec v1, TwoJetVec v2, TwoJet weight) {
  return (v1) * (weight*-1 + 1) + v2*weight;
}

TwoJet Length(TwoJetVec v)
{
  return (TwoJet(v.x^2) + TwoJet(v.y^2)) ^ (.5);
}

// ----------------------------------------

struct ThreeJetVec {
  ThreeJet x;
  ThreeJet y;
  ThreeJet z;
  operator TwoJetVec() { return TwoJetVec(x,y,z); }
};

ThreeJetVec operator+(ThreeJetVec v, ThreeJetVec w);
ThreeJetVec operator*(ThreeJetVec v, ThreeJet  a);
ThreeJetVec operator*(ThreeJetVec v, double a);
ThreeJetVec AnnihilateVec(ThreeJetVec v, int index);
ThreeJetVec Cross(ThreeJetVec v, ThreeJetVec w);
ThreeJet Dot(ThreeJetVec v, ThreeJetVec w);
TwoJetVec D(ThreeJetVec x, int index);
ThreeJetVec Normalize(ThreeJetVec v);
ThreeJetVec RotateZ(ThreeJetVec v, ThreeJet angle);
ThreeJetVec RotateY(ThreeJetVec v, ThreeJet angle);
ThreeJetVec RotateX(ThreeJetVec v, ThreeJet angle);
ThreeJetVec InterpolateVec(ThreeJetVec v1, ThreeJetVec v2, ThreeJet weight);
ThreeJet Length(ThreeJetVec v);

// ----------------------------------------

ThreeJetVec operator+(ThreeJetVec v, ThreeJetVec w) {
  ThreeJetVec result;
  result.x = v.x + w.x;
  result.y = v.y + w.y;
  result.z = v.z + w.z;
  return result;
}

ThreeJetVec operator*(ThreeJetVec v, ThreeJet  a) {
  ThreeJetVec result;
  result.x = v.x*a;
  result.y = v.y*a;
  result.z = v.z*a;
  return result;
}

ThreeJetVec operator*(ThreeJetVec v, double a) {
  ThreeJetVec result;
  result.x = v.x*a;
  result.y = v.y*a;
  result.z = v.z*a;
  return result;
}

ThreeJetVec AnnihilateVec(ThreeJetVec v, int index) {
  ThreeJetVec result;
  result.x = Annihilate(v.x, index);
  result.y = Annihilate(v.y, index);
  result.z = Annihilate(v.z, index);
  return result;
}

TwoJetVec D(ThreeJetVec x, int index) {
  TwoJetVec result;
  result.x = D(x.x, index);
  result.y = D(x.y, index);
  result.z = D(x.z, index);
  return result;
}

ThreeJetVec Cross(ThreeJetVec v, ThreeJetVec w) {
  ThreeJetVec result;
  result.x = v.y*w.z + v.z*w.y*-1;
  result.y = v.z*w.x + v.x*w.z*-1;
  result.z = v.x*w.y + v.y*w.x*-1;
  return result;
}

ThreeJet Dot(ThreeJetVec v, ThreeJetVec w) {
  return v.x*w.x + v.y*w.y + v.z*w.z;
}

ThreeJetVec Normalize(ThreeJetVec v) {
  ThreeJet a;
  a = Dot(v,v);
  if (a > 0)
    a = a^-0.5;
  else
    a = ThreeJet(0, 0, 0);
  return v*a;
}

ThreeJetVec RotateZ(ThreeJetVec v, ThreeJet angle) {
  ThreeJetVec result;
  ThreeJet s,c;
  s = Sin (angle);
  c = Cos (angle);
  result.x =          v.x*c + v.y*s;
  result.y = v.x*s*-1 + v.y*c;
  result.z = v.z;
  return result;
}

ThreeJetVec RotateY(ThreeJetVec v, ThreeJet angle) {
  ThreeJetVec result;
  ThreeJet s, c;
  s = Sin (angle);
  c = Cos (angle);
  result.x = v.x*c + v.z*s*-1;
  result.y = v.y;
  result.z = v.x*s + v.z*c    ;
  return result;
}

ThreeJetVec RotateX(ThreeJetVec v, ThreeJet angle) {
  ThreeJetVec result;
  ThreeJet s,c;
  s = Sin (angle);
  c = Cos (angle);
  result.x = v.x;
  result.y = v.y*c + v.z*s;
  result.z = v.y*s*-1 + v.z*c;
  return result;
}

ThreeJetVec InterpolateVec(ThreeJetVec v1, ThreeJetVec v2, ThreeJet weight) {
  return (v1) * (weight*-1 + 1) + v2*weight;
}

ThreeJet Length(ThreeJetVec v)
{
  return (ThreeJet(v.x^2) + ThreeJet(v.y^2)) ^ (.5);
}

// ----------------------------------------

TwoJetVec FigureEight(TwoJetVec w, TwoJetVec h, TwoJetVec bend, TwoJet form, TwoJet v) {

   TwoJet height;
   v %= 1;
   height = (Cos (v*2) + -1) * (-1);
   if (v > 0.25 && v < 0.75)
      height = height*-1 + 4;
   height = height*0.6;
   h = h + bend*(height*height*(1/64.0));
   return w*Sin (v*2) + (h) * (Interpolate((Cos (v) + -1) * (-2), height, form)) ;
}

TwoJetVec AddFigureEight(ThreeJetVec p, ThreeJet u, TwoJet v, ThreeJet form, ThreeJet scale, int numStrips) {

   ThreeJet size = form * scale;
   form = form*2 + form*form*-1;
   TwoJetVec dv = AnnihilateVec(D(p, 1), 1);
   p = AnnihilateVec(p, 1);
   TwoJetVec du = Normalize(D(p, 0));
   TwoJetVec h = Normalize(Cross(du, dv))*TwoJet(size);
   TwoJetVec w = Normalize(Cross(h, du))*(TwoJet(size)*1.1);
   return RotateZ(
      TwoJetVec(p) +
      FigureEight(w, h, du*D(size, 0)*(D(u, 0)^(-1)), form, v),
      v*(1.0/numStrips)
   );
}

// ----------------------------------------

ThreeJetVec Arc(ThreeJet u, ThreeJet v, double xsize, double ysize, double zsize) {

   ThreeJetVec result;
   u = u*0.25;
   result.x = Sin (u) * Sin (v) * xsize;
   result.y = Sin (u) * Cos (v) * ysize;
   result.z = Cos (u) * zsize;
   return result;
}

ThreeJetVec Straight(ThreeJet u, ThreeJet v, double xsize, double ysize, double zsize) {

   ThreeJetVec result;
   u = u*0.25;
#if 0
   u = (u) * (-0.15915494) + 1; /* 1/2pi */
#endif
   result.x = Sin (v) * xsize;
   result.y = Cos (v) * ysize;
   result.z = Cos (u) * zsize;
   return result;
}

ThreeJet Param1(ThreeJet x) {

   double offset = 0;
   x %= 4;
   if (x > 2) { x = x+(-2); offset = 2; }
   if (x <= 1) return x*2 + (x^2)*(-1) + offset;
   else return (x^2) + x*(-2) + (2 + offset);
}

ThreeJet Param2(ThreeJet x) {

   double offset = 0;
   x %= 4;
   if (x > 2) { x = x+(-2); offset = 2; }
   if (x <= 1) return (x^2) + offset;
   else return (x^2)*(-1) + x*4 + (-2 + offset);
}

static inline ThreeJet TInterp(double x) {
   return ThreeJet(x,0,0);
}

ThreeJet UInterp(ThreeJet x) {

   x %= 2;
   if (x > 1)
      x = x*(-1) + 2;
   return (x^2)*3 + (x^3) * (-2);
}

#define FFPOW 3
ThreeJet FFInterp(ThreeJet x) {

   x %= 2;
   if (x > 1)
      x = x*(-1) + 2;
   x = x*1.06 + -0.05;
   if (x < 0) return ThreeJet(0, 0, 0);
   else if (x > 1) return ThreeJet(0, 0, 0) + 1;
   else return (x ^ (FFPOW-1)) * (FFPOW) + (x^FFPOW) * (-FFPOW+1);
}

#define FSPOW 3
ThreeJet FSInterp(ThreeJet x) {

   x %= 2;
   if (x > 1)
      x = x*(-1) + 2;
   return ((x ^ (FSPOW-1)) * (FSPOW) + (x^FSPOW) * (-FSPOW+1)) * (-0.2);
}

ThreeJetVec Stage0(ThreeJet u, ThreeJet v) {
   return Straight(u, v, 1, 1, 1);
}

ThreeJetVec Stage1(ThreeJet u, ThreeJet v) {
   return Arc(u, v, 1, 1, 1);
}

ThreeJetVec Stage2(ThreeJet u, ThreeJet v) {
   return InterpolateVec(
      Arc(Param1(u), v, 0.9, 0.9, -1),
      Arc(Param2(u), v, 1, 1, 0.5),
      UInterp(u)
   );
}

ThreeJetVec Stage3(ThreeJet u, ThreeJet v) {

   return InterpolateVec(
      Arc(Param1(u), v,-0.9,-0.9,-1),
      Arc(Param2(u), v,-1, 1,-0.5),
      UInterp(u)
   );
}

ThreeJetVec Stage4(ThreeJet u, ThreeJet v) {
   return Arc(u, v, -1,-1, -1);
}

ThreeJetVec Scene01(ThreeJet u, ThreeJet v, double t) {
   return InterpolateVec(Stage0(u,v), Stage1(u,v), TInterp(t));
}

ThreeJetVec Scene12(ThreeJet u, ThreeJet v, double t) {
   return InterpolateVec(Stage1(u,v), Stage2(u,v), TInterp(t));
}

ThreeJetVec Scene23(ThreeJet u, ThreeJet v, double t) {

   ThreeJet tmp = TInterp(t);
   t = tmp.f * 0.5;
   double tt = (u <= 1) ? t : -t;
   return InterpolateVec(
      RotateZ(Arc(Param1(u), v, 0.9, 0.9,-1), ThreeJet(tt,0,0)),
      RotateY(Arc(Param2(u), v, 1, 1, 0.5), ThreeJet(t,0,0)),
      UInterp(u)
  );
}

ThreeJetVec Scene34(ThreeJet u, ThreeJet v, double t) {
   return InterpolateVec(Stage3(u,v), Stage4(u,v), TInterp(t));
}

TwoJetVec BendIn(ThreeJet u, ThreeJet v, double t, int numStrips) {

   ThreeJet tmp = TInterp(t);
   t = tmp.f;
   return AddFigureEight(
      Scene01(u, ThreeJet(0, 0, 1), t),
      u, v, ThreeJet(0, 0, 0), FSInterp(u),
      numStrips
   );
}

TwoJetVec Corrugate(ThreeJet u, ThreeJet v, double t, int numStrips) {

   ThreeJet tmp = TInterp(t);
   t = tmp.f;
   return AddFigureEight(
      Stage1(u, ThreeJet(0, 0, 1)),
      u, v, FFInterp(u) * ThreeJet(t,0,0), FSInterp(u),
      numStrips
   );
}

TwoJetVec PushThrough(ThreeJet u, ThreeJet v, double t, int numStrips) {

   return AddFigureEight(
      Scene12(u,ThreeJet(0, 0, 1),t),
      u, v, FFInterp(u), FSInterp(u),
      numStrips
   );
}

TwoJetVec Twist(ThreeJet u, ThreeJet v, double t, int numStrips) {

   return AddFigureEight(
      Scene23(u,ThreeJet(0, 0, 1),t),
      u, v, FFInterp(u), FSInterp(u),
      numStrips
   );
}

TwoJetVec UnPush(ThreeJet u, ThreeJet v, double t, int numStrips) {

   return AddFigureEight(
      Scene34(u,ThreeJet(0, 0, 1),t),
      u, v, FFInterp(u), FSInterp(u),
      numStrips
   );
}

TwoJetVec UnCorrugate(ThreeJet u, ThreeJet v, double t, int numStrips) {

   ThreeJet tmp;
   tmp = TInterp((t) * (-1) + 1);
   t = tmp.f;

   return AddFigureEight(
      Stage4(u,ThreeJet(0, 0, 1)),
      u, v, FFInterp(u) * ThreeJet(t,0,0), FSInterp(u),
      numStrips
   );
}

// ----------------------------------------

void printMesh(TwoJetVec p, GLPoint * point) {

    double x = p.x.f ;
    double y = p.y.f ;
    double z = p.z.f ;
    double nx = p.y.df_du()*p.z.df_dv()-p.z.df_du()*p.y.df_dv();
    double ny = p.z.df_du()*p.x.df_dv()-p.x.df_du()*p.z.df_dv();
    double nz = p.x.df_du()*p.y.df_dv()-p.y.df_du()*p.x.df_dv();
    double s = nx*nx + ny*ny + nz*nz;
    if (s > 0) s = sqrt(1/s);

    /* printf("%f %f %f    %f %f %f\n", x, y, z, nx*s, ny*s, nz*s); */

    point->vertex[0] = x;
    point->vertex[1] = y;
    point->vertex[2] = z;
    point->normal[0] = -nx*s;
    point->normal[1] = -ny*s;
    point->normal[2] = -nz*s;
}

// ----------------------------------------

typedef TwoJetVec SurfaceTimeFunction(ThreeJet u, ThreeJet v, double t, int numStrips);

static inline double sqr(double x) {
  return x*x;
}
static inline double calcSpeedV(TwoJetVec v) {
  return sqrt(sqr(v.x.df_dv()) + sqr(v.y.df_dv()) + sqr(v.z.df_dv()));
}
static inline double calcSpeedU(TwoJetVec v) {
  return sqrt(sqr(v.x.df_du()) + sqr(v.y.df_du()) + sqr(v.z.df_du()));
}

void printScene(
   SurfaceTimeFunction *func,
   double umin, double umax, int ucount,
   double vmin, double vmax, int vcount,
   double t,
   GLPoint ** geometryMatrix,
   int numStrips
) {
   static TwoJetVec **values;
   int j, k;
   double u, v, delta_u, delta_v;

   if (ucount <= 0 || vcount <= 0) return;
   delta_u = (umax-umin) / ucount;
   delta_v = (vmax-vmin) / vcount;
   values = (TwoJetVec **) calloc(ucount+1, sizeof(TwoJetVec *));
   double *speedv = (double *) calloc(ucount+1, sizeof(double));
   double **speedu = (double **) calloc(ucount+1, sizeof(double *));
   for (j = 0; j <= ucount; j++) {
      u = umin + j*delta_u;
      values[j] = (TwoJetVec *) calloc(vcount+1, sizeof(TwoJetVec));
      speedu[j] = (double *) calloc(vcount+1, sizeof(double));
      speedv[j] = calcSpeedV((*func)(ThreeJet(u, 1, 0), ThreeJet(0, 0, 1), t, numStrips));
      if (speedv[j] == 0) {
         /* Perturb a bit, hoping to avoid degeneracy */
         u += (u < 1) ? 1e-9 : -1e-9;
         speedv[j] = calcSpeedV((*func)(ThreeJet(u, 1, 0), ThreeJet(0, 0, 1), t, numStrips));
      }
      for (k = 0; k <= vcount; k++) {
         v = vmin + k*delta_v;
         values[j][k] = (*func)( ThreeJet(u, 1, 0), ThreeJet(v, 0, 1), t, numStrips );
         speedu[j][k] = calcSpeedU(values[j][k]);
      }
   }

   /* quadrilateral mesh code */

   for (j = 0; j <= ucount; ++j)
      for (k = 0; k <= vcount; ++k) {
         printMesh(values[j][k],  &geometryMatrix[j][k]);
      }

   /* clean up */
   for (j = 0; j <= ucount; j++) {
      free(values[j]);
      free(speedu[j]);
   }
   free(values);
   free(speedu);
   free(speedv);
}

// ----------------------------------------

/*
   Refer to generateGeometry.h for
   documentation on this function.
*/
void generateGeometry(
   GLPoint ** geometryMatrix,
   double time,
   int numStrips,

   double u_min,
   int u_count,
   double u_max,
   double v_min,
   int v_count,
   double v_max,

   double bendtime,

   double corrStart,
   double pushStart,
   double twistStart,
   double unpushStart,
   double uncorrStart
) {
   if (NULL == geometryMatrix)
      return;

   if (bendtime >= 0.0) {
      printScene(BendIn, u_min, u_max, u_count, v_min, v_max, v_count, bendtime, geometryMatrix, numStrips );
   } else {

      /* time = (time - howfar) / chunk */

      if (time >= uncorrStart)
         printScene(UnCorrugate, u_min, u_max, u_count, v_min, v_max, v_count,
		   (time - uncorrStart) / (1.0 - uncorrStart), geometryMatrix, numStrips );
      else if (time >= unpushStart)
         printScene(UnPush, u_min, u_max, u_count, v_min, v_max, v_count,
		   (time - unpushStart) / (uncorrStart - unpushStart), geometryMatrix, numStrips );
      else if (time >= twistStart)
         printScene(Twist, u_min, u_max, u_count, v_min, v_max, v_count,
		   (time - twistStart) / (unpushStart - twistStart), geometryMatrix, numStrips );
      else if (time >= pushStart)
         printScene(PushThrough, u_min, u_max, u_count, v_min, v_max, v_count,
		   (time - pushStart) / (twistStart - pushStart), geometryMatrix, numStrips );
      else if (time >= corrStart)
         printScene(Corrugate, u_min, u_max, u_count, v_min, v_max, v_count,
		   (time - corrStart) / (pushStart - corrStart), geometryMatrix, numStrips );
   }
}

