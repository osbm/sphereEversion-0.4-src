
#ifndef MATHUTIL_H
#define MATHUTIL_H


#include "global.h"
#include <vector>
using namespace std;
#include <math.h>


#ifdef DEBUG
   /* convenient macro for printing vectors and points during debugging */
   #define PRINT(v) printf( "%f,%f,%f\n", (v).x(), (v).y(), (v).z() );

   /* convenient macro for printing matrices during debugging */
   #define PRINT_MATRIX(m) printf( \
      "%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n--\n", \
      m( 0), m( 4), m( 8), m(12), \
      m( 1), m( 5), m( 9), m(13), \
      m( 2), m( 6), m(10), m(14), \
      m( 3), m( 7), m(11), m(15) \
   );

   #define ASSERT_IS_NORMALIZED(v) { \
      ASSERT_IS_EQUAL(v.length(),1) \
   }
#else
   #define ASSERT_IS_NORMALIZED(v)
#endif


class Point3;
class Vector3;
Vector3 operator+( const Vector3&, const Vector3& );
Vector3 operator-( const Point3&, const Point3& );
Point3 operator+( const Point3&, const Vector3& );


// Why are the below vector and point classes homogeneous
// (i.e. they have a 4th component) ?
// Admittedly, the 4th component is usually equal to 1,
// and thus wastes CPU time during computations.
// However, having all points and vectors homogeneous
// makes it simpler to work with 4x4 matrices,
// and it also guarantees well-defined behaviour when
// we call a gl*() routine and pass in vector.get()
// or point.get(), whether the routine expects an array
// of 3 components or 4.
// For example, glLightfv(GL_LIGHT0,GL_POSITION,point.get())
// expects an array of 4 elements to be passed in as the 3rd argument.
// If you pass in an array of 3 elements, you can get strange behaviour
// that can take a long time to track down.
//
// Update: it seems now that, for vectors, w should be zero.  FIXME
// See for example page 10 of
// "Realistic Ray Tracing" by Peter Shirley (2000).


class Vector3 {
private:
   float _v[4];
public:
   Vector3() { _v[0] = _v[1] = _v[2] = 0;  _v[3] = 1; }
   Vector3( float x, float y, float z, float w = 1 ) {
      _v[0] = x;  _v[1] = y;  _v[2] = z;  _v[3] = w;
   }

   // copy
   Vector3( const Vector3& v ) {
      _v[0] = v.x();  _v[1] = v.y();  _v[2] = v.z();  _v[3] = v.w();
   }
   Vector3& operator=( const Vector3& v ) {
      _v[0] = v.x();  _v[1] = v.y();  _v[2] = v.z();  _v[3] = v.w();
      return *this;
   }

   // type conversion
   explicit inline Vector3( const Point3& );

   float x() const { return _v[0]; }
   float y() const { return _v[1]; }
   float z() const { return _v[2]; }
   float w() const { return _v[3]; }
   float& x() { return _v[0]; }
   float& y() { return _v[1]; }
   float& z() { return _v[2]; }
   float& w() { return _v[3]; }

   // used to pass coordinates directly to OpenGL routines
   const float * get() const { return _v; }

   // Can be used to directly modify a component of the vector
   // using an index that, for example, was returned
   // by something like indexOfLeastComponent() called on another vector.
   float * get() { return _v; }

   float lengthSquared() const { return x()*x() + y()*y() + z()*z(); }

   double length() const { return sqrt( lengthSquared() ); }

   Vector3 normalized() const;

   Vector3 choosePerpendicular() const;

   // Returns one of 0,1,2, for x,y,z respectively.
   // (If there is a tie, the smaller index is returned.)
   // Can be interpreted as the axis-aligned plane that
   // is most parallel to the vector
   // (e.g. if 0 is returned, then the yz plane is most parallel).
   // Note that the returned value can be used to index into
   // the array returned by get().
   short indexOfLeastComponent() const;

   // Returns one of 0,1,2, for x,y,z respectively.
   // (If there is a tie, the smaller index is returned.)
   // Can be interpreted as the axis-aligned plane that
   // is most perpendicular to the vector
   // (e.g. if 0 is returned, then the yz plane is most perpendicular).
   // Note that the returned value can be used to index into
   // the array returned by get().
   short indexOfGreatestComponent() const;

   inline Vector3 operator-() const {
      return Vector3( -x(), -y(), -z(), w() );
   }
   Vector3 operator*( float k ) const {
      return Vector3( k*x(), k*y(), k*z() );
   }
   Vector3 operator+=( const Vector3& v ) {
      _v[0] += v.x();  _v[1] += v.y();  _v[2] += v.z();
      ASSERT( v.w() == 1 );
      return *this;
   }

   // Returns the angle, in [0,pi], between the two given vectors.
   static float computeAngle( const Vector3 & v1, const Vector3 & v2 );

   // Returns the angle, in [-pi,pi], between the two given vectors,
   // and whose sign corresponds to the right-handed rotation around
   // ``axis'' to get from v1 to v2.
   static float computeSignedAngle(
      const Vector3 & v1, const Vector3 & v2, const Vector3 & axis
   );
};


class Point3 {
private:
   float _p[4];
public:
   Point3() { _p[0] = _p[1] = _p[2] = 0;  _p[3] = 1; }
   Point3( float x, float y, float z, float w = 1 ) {
      _p[0] = x;  _p[1] = y;  _p[2] = z;  _p[3] = w;
   }

   // copy
   Point3( const Point3& p ) {
      _p[0] = p.x();  _p[1] = p.y();  _p[2] = p.z();  _p[3] = p.w();
   }
   Point3& operator=( const Point3& p ) {
      _p[0] = p.x();  _p[1] = p.y();  _p[2] = p.z();  _p[3] = p.w();
      return *this;
   }

   // type conversion
   explicit inline Point3( const Vector3& );

   float x() const { return _p[0]; }
   float y() const { return _p[1]; }
   float z() const { return _p[2]; }
   float w() const { return _p[3]; }
   float& x() { return _p[0]; }
   float& y() { return _p[1]; }
   float& z() { return _p[2]; }
   float& w() { return _p[3]; }

   // used to pass coordinates directly to OpenGL routines
   const float * get() const { return _p; }

   Point3 operator+=( const Vector3& v ) {
      _p[0] += v.x();  _p[1] += v.y();  _p[2] += v.z();
      ASSERT( v.w() == 1 );
      return *this;
   }
   Point3 operator-=( const Vector3& v ) {
      _p[0] -= v.x();  _p[1] -= v.y();  _p[2] -= v.z();
      ASSERT( v.w() == 1 );
      return *this;
   }
};


class Matrix {
private:
   float _m[16];
public:
   Matrix() { setToIdentity(); }
   void setToIdentity();

   // Apparently, there is no need to define a
   // copy constructor or assignment operator

   float operator()( int i ) const { return _m[i]; }
   float& operator()( int i ) { return _m[i]; }

   // used to pass coordinates directly to OpenGL routines
   const float * get() const { return _m; }

   void transpose();

   void setToTranslation( const Vector3& );

   void setToScale( const Vector3& );

   // Scale with respect to the given frame of reference.
   void setToScale(
      // The scaling is with respect to this origin and frame.
      const Point3 & origin,
      const Vector3 & i, const Vector3 & j, const Vector3 & k,

      // The components of this vector correspond to
      // the i,j,k directions respectively
      const Vector3 & scaleFactor
   );

   // The angle must be in [-pi,pi], and corresponds to a counterclockwise
   // rotation around v, with v pointing up at the observer.
   // Also, v must be normalized.
   void setToRotation( float angle_in_radians, const Vector3& v );

   // Allows rotation around any axis, rather than just around the origin.
   // Rotates around the line ``origin + v*t'' where t is a real number.
   void setToRotation(
      float angle_in_radians, const Vector3& v, const Point3 & origin
   );

   // Constructs a rotation matrix from the 3 given vectors
   void setToRotation(
      const Vector3 & i, const Vector3 & j, const Vector3 & k
   );

   // Generate a view matrix.
   // The line of sight corresponds to the -z axis,
   // and the up direction corresponds to the +y axis.
   // Note that a look-at matrix converts world space coordinates
   // to coordinates in the local frame of reference,
   // and can be used for setting up the view in OpenGL.
   // An inverse look-at matrix converts coordinates from
   // local space to world space, and can be used for
   // rendering a model in its local space.
   void setToLookAt(
      const Point3& eye, const Point3& target, const Vector3& up,
      bool inverted = false
   );

   // Generate a perspective projection matrix.
   void setToFrustum(
      float left, float right, float bottom, float top,
      float nearPlane, float farPlane,
      bool inverted = false
   );
};


class Ray {
public:
   Point3 origin;
   Vector3 direction;  // a unit vector
   Ray() {}
   Ray( const Point3& o, const Vector3& d ) : origin(o), direction(d) {
      ASSERT_IS_NORMALIZED( direction );
   }
   Point3 point( float t ) const { return origin + direction * t; }
};


class AlignedBox {
private:
   bool _isEmpty;
   Point3 _p0, _p1;
public:
   AlignedBox() : _isEmpty( true ) {}
   AlignedBox( const Point3& min, const Point3& max ) :
      _isEmpty( false ), _p0( min ), _p1( max )
   {
      ASSERT( _p0.x() <= _p1.x() );
      ASSERT( _p0.y() <= _p1.y() );
      ASSERT( _p0.z() <= _p1.z() );
   }
   void clear() { _isEmpty = true; }
   void bound( const Point3& );
   void bound( const vector< Point3 >&, const Matrix& );
   void bound( const AlignedBox& );

   bool isEmpty() const { return _isEmpty; }
   bool contains( const Point3& p ) const {
      return
         _p0.x() <= p.x() && p.x() <= _p1.x()
         && _p0.y() <= p.y() && p.y() <= _p1.y()
         && _p0.z() <= p.z() && p.z() <= _p1.z();
   }
   const Point3& getMin() const { return _p0; }
   const Point3& getMax() const { return _p1; }
   Vector3 getDiagonal() const { return _p1 - _p0; }
   Point3 getCentre() const {
      return Point3( (Vector3(_p0)+Vector3(_p1))*0.5f );
   }

   // Returns corner (or the index of the corner)
   // that is furthest in the direction of the given vector.
   Point3 getExtremeCorner( const Vector3 & v ) const {
      return Point3(
         v.x() > 0 ? _p1.x() : _p0.x(),
         v.y() > 0 ? _p1.y() : _p0.y(),
         v.z() > 0 ? _p1.z() : _p0.z()
      );
   }
   int getIndexOfExtremeCorner( const Vector3 & v ) const {
      return (v.x() > 0) | ((v.y() > 0)<<1) | ((v.z() > 0)<<2);
   }

   Point3 getCorner( int i ) const {
      return Point3(
         i & 1 ? _p1.x() : _p0.x(),
         i & 2 ? _p1.y() : _p0.y(),
         i & 4 ? _p1.z() : _p0.z()
      );
   }

   // Returns false if no intersection;
   // returns true if there *may* be an intersection.
   // This is a good test to use if the client only
   // needs an approximate test, because it's fast.
   bool intersects( const Ray& ) const;

   // Returns true if there is an intersection,
   // in which case the point of intersection is also returned.
   // This is an exact test.
   bool intersectsExactly( const Ray & ray, Point3 & intersection );
};


class Plane {
private:
   Vector3 _n; // the normal
   float _d;//equation of the plane is ax+by+cz+d=0, where (a,b,c) is the normal
public:
   Plane() : _d(0) { }
   Plane( const Vector3& normal, const Point3& p ) :
      _n( normal.normalized() ) {
         _d = - _n.x()*p.x() - _n.y()*p.y() - _n.z()*p.z();
      }
   //Plane( const Point3& p0, const Point3& p1, const Point3& p2 );
   //Plane( const Ray& ray, const Point3& p );

   // Returns true if there is an intersection,
   // in which case the point of intersection is also returned
   bool intersects( const Ray & ray, Point3 & intersection,
      bool allowIntersectionEvenIfPlaneIsBackfacing = true
   );

   // Returns +1 or -1 if the argument is completely on one side of the plane
   // (positive if it's in the direction of the normal,
   // negative for the other side),
   // or zero if the argument intersects the plane.
   int side( const Point3& p ) const {
      float s = _n.x()*p.x() + _n.y()*p.y() + _n.z()*p.z() + _d;
      if ( s == 0 ) return 0;
      return s < 0 ? -1 : 1;
   }
   int side( const AlignedBox& b ) const {
      // get index of box's corner furthest in direction of plane's normal
      int i = b.getIndexOfExtremeCorner( _n );

      int s0 = side( b.getCorner( i ) );
      // "i^7" is the index of the opposite corner
      int s1 = side( b.getCorner( i^7 ) );

      int s = s0 + s1;
      if ( s == 0 ) return 0;
      return s < 0 ? -1 : 1;
   }
};


class Sphere {
private:
   Point3 _centre;
   float _radiusSquared;
public:
   Sphere( const Point3 & centre, float radius ) :
      _centre( centre ), _radiusSquared( radius * radius )
   { }
   bool intersects( const Ray & ray, Point3 & intersection,
      bool allowIntersectionEvenIfRayOriginatesInsideSphere = true
   );
};


inline Vector3::Vector3( const Point3& p ) {
   _v[0] = p.x();  _v[1] = p.y();  _v[2] = p.z();  _v[3] = p.w();
}
inline Point3::Point3( const Vector3& v ) {
   _p[0] = v.x();  _p[1] = v.y();  _p[2] = v.z();  _p[3] = v.w();
}
inline Vector3 operator+( const Vector3& v1, const Vector3& v2 ) {
   ASSERT( v1.w() == 1 && v2.w() == 1 );
   return Vector3( v1.x()+v2.x(), v1.y()+v2.y(), v1.z()+v2.z() );
}
inline float operator*( const Vector3& v1, const Vector3& v2 ) {
   ASSERT( v1.w() == 1 && v2.w() == 1 );
   return v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
}
inline Point3 operator+( const Point3& p, const Vector3& v ) {
   return Point3( p.x()+v.x(), p.y()+v.y(), p.z()+v.z() );
}
inline Vector3 operator-( const Point3& p1, const Point3& p2 ) {
   return Vector3( p1.x()-p2.x(), p1.y()-p2.y(), p1.z()-p2.z() );
}
inline Point3 operator-( const Point3& p, const Vector3& v ) {
   return Point3( p.x()-v.x(), p.y()-v.y(), p.z()-v.z() );
}
inline Vector3 operator^( const Vector3& v1,  const Vector3& v2 ) {
   return Vector3(
      v1.y()*v2.z() - v1.z()*v2.y(),
      v1.z()*v2.x() - v1.x()*v2.z(),
      v1.x()*v2.y() - v1.y()*v2.x()
   );
}
Vector3 operator*( const Matrix&, const Vector3& );
inline Point3 operator*( const Matrix& m, const Point3& p ) {
   return Point3( m * Vector3( p ) );
}
Matrix operator*( const Matrix&, const Matrix& );


class LineSegment {
public:
   int A_x, A_y, B_x, B_y;
   float distanceToPointSquared( int P_x, int P_y ) const;

   // Computes the component of the given vector in the
   // direction of the line segment, and divides by the
   // length of the line segment.
   // Equivalent to the dot product divided by the length squared
   // of the line segment, i.e. (B-A)*(dx,dy) / |(B-A)|^2
   float scaledComponent( int dx, int dy ) const;
};


#endif /* MATHUTIL_H */

