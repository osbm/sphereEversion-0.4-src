
#include "mathutil.h"


Vector3 Vector3::normalized() const {

   double l = length();
   return l > 0
      ? ((*this) * (1/l))
      : (*this);
}

Vector3 Vector3::choosePerpendicular() const {

   // strategy: pick the two largest components,
   // permute them and negate one of them, and
   // replace the other (i.e. smallest) component with zero.

   float X = fabs(x());
   float Y = fabs(y());
   float Z = fabs(z());

   Vector3 v;

   if ( X < Y ) {
      if ( Y < Z ) {
         // X < Y < Z
         v = Vector3( 0, z(), -y() );
      }
      else if ( X < Z ) {
         // X < Z <= Y
         v = Vector3( 0, z(), -y() );
      }
      else {
         // Z <= X < Y
         v = Vector3( y(), -x(), 0 );
      }
   }
   else {
      if ( Z < Y ) {
         // Z < Y <= X
         v = Vector3( y(), -x(), 0 );
      }
      else if ( Z < X ) {
         // Y <= Z < X
         v = Vector3( z(), 0, -x() );
      }
      else {
         // Y <= X <= Z
         v = Vector3( z(), 0, -x() );
      }
   }

   #ifdef DEBUG
   float dotProduct = v * (*this);
   ASSERT_IS_EQUAL( dotProduct, 0 );
   #endif

   return v;
}

short Vector3::indexOfLeastComponent() const {
   float X = fabs(x());
   float Y = fabs(y());
   float Z = fabs(z());
   if ( Y <= Z ) {
      return X <= Y ? 0 : 1;
   }
   else {
      return X <= Z ? 0 : 2;
   }
}

short Vector3::indexOfGreatestComponent() const {
   float X = fabs(x());
   float Y = fabs(y());
   float Z = fabs(z());
   if ( Y >= Z ) {
      return X >= Y ? 0 : 1;
   }
   else {
      return X >= Z ? 0 : 2;
   }
}

float Vector3::computeAngle( const Vector3 & v1, const Vector3 & v2 ) {
   Vector3 cross = v1.normalized() ^ v2.normalized();

   // Due to numerical inaccuracies, the length of the cross product
   // may be slightly more than 1.
   // Calling arcsin on such a value would result in NaN.
   float lengthOfCross = cross.length();
   float angle = ( lengthOfCross >= 1 ) ? M_PI/2 : asin( lengthOfCross );

   if ( v1 * v2 < 0 )
      angle = M_PI - angle;
   return angle;
}

float Vector3::computeSignedAngle(
   const Vector3 & v1, const Vector3 & v2, const Vector3 & axis
) {
   Vector3 cross = v1.normalized() ^ v2.normalized();

   // Due to numerical inaccuracies, the length of the cross product
   // may be slightly more than 1.
   // Calling arcsin on such a value would result in NaN.
   float lengthOfCross = cross.length();
   float angle = ( lengthOfCross >= 1 ) ? M_PI/2 : asin( lengthOfCross );

   if ( v1 * v2 < 0 )
      angle = M_PI - angle;
   if ( cross*axis < 0 ) angle = -angle;
   return angle;
}



void Matrix::setToIdentity() {
   _m[ 0] = 1; _m[ 4] = 0; _m[ 8] = 0; _m[12] = 0;
   _m[ 1] = 0; _m[ 5] = 1; _m[ 9] = 0; _m[13] = 0;
   _m[ 2] = 0; _m[ 6] = 0; _m[10] = 1; _m[14] = 0;
   _m[ 3] = 0; _m[ 7] = 0; _m[11] = 0; _m[15] = 1;
}

void Matrix::transpose() {
   float tmp;
   #define SWAP(a,b) (tmp)=(a); (a)=(b); (b)=(tmp);
   SWAP(_m[ 1],_m[ 4]);
   SWAP(_m[ 2],_m[ 8]);
   SWAP(_m[ 3],_m[12]);
   SWAP(_m[ 7],_m[13]);
   SWAP(_m[11],_m[14]);
   SWAP(_m[ 6],_m[ 9]);
   #undef SWAP
}

void Matrix::setToTranslation( const Vector3& v ) {
   _m[ 0] = 1; _m[ 4] = 0; _m[ 8] = 0; _m[12] = v.x();
   _m[ 1] = 0; _m[ 5] = 1; _m[ 9] = 0; _m[13] = v.y();
   _m[ 2] = 0; _m[ 6] = 0; _m[10] = 1; _m[14] = v.z();
   _m[ 3] = 0; _m[ 7] = 0; _m[11] = 0; _m[15] = 1;
}

void Matrix::setToScale( const Vector3& v ) {
   _m[ 0] = v.x(); _m[ 4] = 0;     _m[ 8] = 0;     _m[12] = 0;
   _m[ 1] = 0;     _m[ 5] = v.y(); _m[ 9] = 0;     _m[13] = 0;
   _m[ 2] = 0;     _m[ 6] = 0;     _m[10] = v.z(); _m[14] = 0;
   _m[ 3] = 0;     _m[ 7] = 0;     _m[11] = 0;     _m[15] = 1;
}

void Matrix::setToScale(
   const Point3 & origin,
   const Vector3 & i, const Vector3 & j, const Vector3 & k,
   const Vector3 & scaleFactor
) {
   Matrix translationMatrix, rotationMatrix;
   translationMatrix.setToTranslation( - Vector3( origin ) );
   rotationMatrix.setToRotation( i, j, k );
   setToScale( scaleFactor );
   (*this) = (*this) * rotationMatrix * translationMatrix;
   rotationMatrix.transpose(); // equivalent to inverting the matrix
   translationMatrix.setToTranslation( Vector3( origin ) );
   (*this) = translationMatrix * rotationMatrix * (*this);
}

void Matrix::setToRotation(
   float angle_in_radians, const Vector3& v
) {
   ASSERT_IS_NORMALIZED(v);
   double c = cos( angle_in_radians );
   double s = sin( angle_in_radians );
   double one_minus_c = 1-c;
   _m[ 0] = c + one_minus_c * v.x()*v.x();
   _m[ 5] = c + one_minus_c * v.y()*v.y();
   _m[10] = c + one_minus_c * v.z()*v.z();
   _m[ 1] = _m[ 4] = one_minus_c * v.x()*v.y();
   _m[ 2] = _m[ 8] = one_minus_c * v.x()*v.z();
   _m[ 6] = _m[ 9] = one_minus_c * v.y()*v.z();
   float xs = v.x() * s;
   float ys = v.y() * s;
   float zs = v.z() * s;
   _m[ 1] += zs;  _m[ 4] -= zs;
   _m[ 2] -= ys;  _m[ 8] += ys;
   _m[ 6] += xs;  _m[ 9] -= xs;

   _m[12] = 0;
   _m[13] = 0;
   _m[14] = 0;
   _m[ 3] = 0; _m[ 7] = 0; _m[11] = 0; _m[15] = 1;
}

void Matrix::setToRotation(
   float angle_in_radians, const Vector3& v, const Point3 & origin
) {
   Matrix tmp;
   tmp.setToTranslation( - Vector3( origin ) );
   setToRotation( angle_in_radians, v );
   (*this) = (*this) * tmp;
   tmp.setToTranslation( Vector3( origin ) );
   (*this) = tmp * (*this);
}

void Matrix::setToRotation(
   const Vector3 & i, const Vector3 & j, const Vector3 & k
) {
   _m[ 0] = i.x();  _m[ 4] = i.y();  _m[ 8] = i.z();
   _m[ 1] = j.x();  _m[ 5] = j.y();  _m[ 9] = j.z();
   _m[ 2] = k.x();  _m[ 6] = k.y();  _m[10] = k.z();

   _m[12] = 0;
   _m[13] = 0;
   _m[14] = 0;
   _m[ 3] = 0; _m[ 7] = 0; _m[11] = 0; _m[15] = 1;
}

void Matrix::setToLookAt(
   const Point3& eye, const Point3& target, const Vector3& up,
   bool inverted
) {
   // step one: generate a rotation matrix

   Vector3 z = (eye - target).normalized();
   Vector3 y = up;
   Vector3 x = y ^ z;
   y = z ^ x;

   // Cross product gives area of parallelogram, which is < 1 for
   // non-perpendicular unit-length vectors; so normalize x and y.
   x = x.normalized();
   y = y.normalized();

   Matrix m2;

   if ( inverted ) {
      // the rotation matrix
      _m[ 0] = x.x(); _m[ 4] = y.x(); _m[ 8] = z.x(); _m[12] = 0;
      _m[ 1] = x.y(); _m[ 5] = y.y(); _m[ 9] = z.y(); _m[13] = 0;
      _m[ 2] = x.z(); _m[ 6] = y.z(); _m[10] = z.z(); _m[14] = 0;
      _m[ 3] = 0;     _m[ 7] = 0;     _m[11] = 0;     _m[15] = 1;

      // step two: premultiply by a translation matrix
      m2.setToTranslation( Vector3(eye) );
      *this = m2 * (*this);
   }
   else {
      // the rotation matrix
      _m[ 0] = x.x(); _m[ 4] = x.y(); _m[ 8] = x.z(); _m[12] = 0;
      _m[ 1] = y.x(); _m[ 5] = y.y(); _m[ 9] = y.z(); _m[13] = 0;
      _m[ 2] = z.x(); _m[ 6] = z.y(); _m[10] = z.z(); _m[14] = 0;
      _m[ 3] = 0;     _m[ 7] = 0;     _m[11] = 0;     _m[15] = 1;

      // step two: tack on a translation matrix
      m2.setToTranslation( -Vector3(eye) );
      *this = (*this) * m2;
   }
}

void Matrix::setToFrustum(
   float left, float right, float bottom, float top,
   float nearPlane, float farPlane,
   bool inverted
) {
   if ( inverted ) {
      float one_over_2n = 0.5f / nearPlane;
      float one_over_2fn = one_over_2n / farPlane;

      // first row
      _m[ 0] = (right-left) * one_over_2n;
      _m[ 4] = 0;
      _m[ 8] = 0;
      _m[12] = (right+left) * one_over_2n;

      // second row
      _m[ 1] = 0;
      _m[ 5] = (top-bottom) * one_over_2n;
      _m[ 9] = 0;
      _m[13] = (top+bottom) * one_over_2n;

      // third row
      _m[ 2] = 0;
      _m[ 6] = 0;
      _m[10] = 0;
      _m[14] = -1;

      // fourth row
      _m[ 3] = 0;
      _m[ 7] = 0;
      _m[11] = (nearPlane-farPlane) * one_over_2fn;
      _m[15] = (nearPlane+farPlane) * one_over_2fn;
   }
   else {
      float two_n = 2 * nearPlane;
      float one_over_width = 1 / ( right - left );
      float one_over_height = 1 / ( top - bottom );
      float one_over_thickness = 1 / ( farPlane - nearPlane );

      // first row
      _m[ 0] = two_n * one_over_width;
      _m[ 4] = 0;
      _m[ 8] = (right+left) * one_over_width;
      _m[12] = 0;

      // second row
      _m[ 1] = 0;
      _m[ 5] = two_n * one_over_height;
      _m[ 9] = (top+bottom) * one_over_height;
      _m[13] = 0;

      // third row
      _m[ 2] = 0;
      _m[ 6] = 0;
      _m[10] = - (farPlane+nearPlane) * one_over_thickness;
      _m[14] = - two_n * farPlane * one_over_thickness;

      // fourth row
      _m[ 3] = 0;
      _m[ 7] = 0;
      _m[11] = -1;
      _m[15] = 0;
   }
}


void AlignedBox::bound( const Point3& p ) {
   if ( _isEmpty ) {
      _p0 = _p1 = p;
      _isEmpty = false;
   }
   else {
      if ( p.x() < _p0.x() ) _p0.x() = p.x();
      else if ( p.x() > _p1.x() ) _p1.x() = p.x();

      if ( p.y() < _p0.y() ) _p0.y() = p.y();
      else if ( p.y() > _p1.y() ) _p1.y() = p.y();

      if ( p.z() < _p0.z() ) _p0.z() = p.z();
      else if ( p.z() > _p1.z() ) _p1.z() = p.z();
   }
}

void AlignedBox::bound( const vector< Point3 >& l, const Matrix& m ) {
   // The naive way to bound a set of points is to loop through
   // them one-by-one, calling bound() on each point.
   // That would require up to 6 comparisons for each point.
   // Below, we use a slightly smarter method that requires
   // 9 comparisons per *pair* of points, hence it's 25 % less work.

   vector< Point3 >::const_iterator it = l.begin();
   if ( l.size() < 3 ) {
      for ( ; it != l.end(); ++it ) {
         bound( m * (*it) );
      }
   }
   else {
      // If we have lots of points, it's efficient to gather
      // up the min and max for each component, handling *two*
      // points at a time.

      // To avoid having to deal a special case within the loop,
      // we must ensure that the bounding box is not empty
      // (possibly by processing an initial point).
      // To ensure the loop can process the points in pairs,
      // we must also ensure that there is an even number of
      // points left to process (again, possibly by processing
      // an initial point).
      if ( l.size() % 2 == 0 ) {
         if ( _isEmpty ) {
            bound( m * (*it) );
            ++it;
            bound( m * (*it) );
            ++it;
         }
      }
      else {
         bound( m * (*it) );
         ++it;
      }

      ASSERT( ! _isEmpty );

      // Now we loop through the remaining points, processing two at a time.
      Point3 a, b;
      while ( it != l.end() ) {
         a = m * (*it);
         ++it;
         ASSERT( it != l.end() );
         b = m * (*it);
         ++it;

         if ( a.x() < b.x() ) {
            if ( a.x() < _p0.x() ) _p0.x() = a.x();
            if ( b.x() > _p1.x() ) _p1.x() = b.x();
         }
         else {
            if ( b.x() < _p0.x() ) _p0.x() = b.x();
            if ( a.x() > _p1.x() ) _p1.x() = a.x();
         }

         if ( a.y() < b.y() ) {
            if ( a.y() < _p0.y() ) _p0.y() = a.y();
            if ( b.y() > _p1.y() ) _p1.y() = b.y();
         }
         else {
            if ( b.y() < _p0.y() ) _p0.y() = b.y();
            if ( a.y() > _p1.y() ) _p1.y() = a.y();
         }

         if ( a.z() < b.z() ) {
            if ( a.z() < _p0.z() ) _p0.z() = a.z();
            if ( b.z() > _p1.z() ) _p1.z() = b.z();
         }
         else {
            if ( b.z() < _p0.z() ) _p0.z() = b.z();
            if ( a.z() > _p1.z() ) _p1.z() = a.z();
         }
      }
   }
}

void AlignedBox::bound( const AlignedBox& b ) {
   if ( ! b.isEmpty() ) {
      ASSERT( b._p0.x() <= b._p1.x() );
      ASSERT( b._p0.y() <= b._p1.y() );
      ASSERT( b._p0.z() <= b._p1.z() );
      if ( _isEmpty ) {
         _p0 = b._p0;
         _p1 = b._p1;
         _isEmpty = false;
      }
      else {
         if ( b._p0.x() < _p0.x() ) _p0.x() = b._p0.x();
         if ( b._p1.x() > _p1.x() ) _p1.x() = b._p1.x();

         if ( b._p0.y() < _p0.y() ) _p0.y() = b._p0.y();
         if ( b._p1.y() > _p1.y() ) _p1.y() = b._p1.y();

         if ( b._p0.z() < _p0.z() ) _p0.z() = b._p0.z();
         if ( b._p1.z() > _p1.z() ) _p1.z() = b._p1.z();
      }
   }
}

bool AlignedBox::intersects( const Ray& ray ) const {
   // We compute a bounding sphere for the box.
   // If the ray intersects the bounding sphere,
   // it *may* intersect the box.
   Point3 intersection;
   return Sphere( getCentre(), (_p1-_p0).length() / 2 ).intersects(
      ray, intersection
   );
}

bool AlignedBox::intersectsExactly( const Ray & ray, Point3 & intersection ) {
   bool intersectionDetected = false;
   float distance = 0;

   // candidate intersection
   float candidateDistance;
   Point3 candidatePoint;

   if ( ray.direction.x() != 0 ) {
      candidateDistance = -(ray.origin.x() - _p0.x())/ray.direction.x();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.y()<=candidatePoint.y() && candidatePoint.y()<=_p1.y()
            && _p0.z()<=candidatePoint.z() && candidatePoint.z()<=_p1.z() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
      candidateDistance = -(ray.origin.x() - _p1.x())/ray.direction.x();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.y()<=candidatePoint.y() && candidatePoint.y()<=_p1.y()
            && _p0.z()<=candidatePoint.z() && candidatePoint.z()<=_p1.z() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
   }
   if ( ray.direction.y() != 0 ) {
      candidateDistance = -(ray.origin.y() - _p0.y())/ray.direction.y();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.x()<=candidatePoint.x() && candidatePoint.x()<=_p1.x()
            && _p0.z()<=candidatePoint.z() && candidatePoint.z()<=_p1.z() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
      candidateDistance = -(ray.origin.y() - _p1.y())/ray.direction.y();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.x()<=candidatePoint.x() && candidatePoint.x()<=_p1.x()
            && _p0.z()<=candidatePoint.z() && candidatePoint.z()<=_p1.z() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
   }
   if ( ray.direction.z() != 0 ) {
      candidateDistance = -(ray.origin.z() - _p0.z())/ray.direction.z();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.y()<=candidatePoint.y() && candidatePoint.y()<=_p1.y()
            && _p0.x()<=candidatePoint.x() && candidatePoint.x()<=_p1.x() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
      candidateDistance = -(ray.origin.z() - _p1.z())/ray.direction.z();
      if (
         distance>=0
         && (!intersectionDetected || candidateDistance<distance)
      ) {
         candidatePoint = ray.origin + ray.direction * candidateDistance;
         if (_p0.y()<=candidatePoint.y() && candidatePoint.y()<=_p1.y()
            && _p0.x()<=candidatePoint.x() && candidatePoint.x()<=_p1.x() ) {
            distance = candidateDistance;
            intersection = candidatePoint;
            intersectionDetected = true;
         }
      }
   }
   return intersectionDetected;
}


bool Plane::intersects(
   const Ray & ray,
   Point3 & intersection,
   bool allowIntersectionEvenIfPlaneIsBackfacing
) {
   float dot = _n * ray.direction;
   if ( !allowIntersectionEvenIfPlaneIsBackfacing && dot > 0 ) {
      return false;
   }
   if ( dot == 0 ) {
      return false;
   }

   // See Foley and van Dam, pg 1101
   Point3 pointOnPlane( _n * (-_d) );
   float t = ( pointOnPlane - ray.origin ) * _n / dot;

   if ( t < 0 ) {
      return false;
   }

   intersection = ray.point( t );
   return true;
}


bool Sphere::intersects(
   const Ray & ray,
   Point3 & intersection,
   bool allowIntersectionEvenIfRayOriginatesInsideSphere
) {
   // a line of the form Q+v*t, where t is real
   const Point3& Q = ray.origin;
   const Vector3& v = ray.direction;

   // the sphere's centre
   const Point3& P = _centre;

   ASSERT_IS_NORMALIZED( v );
   // see Foley and van Dam, pg 1101
   Vector3 Q_minus_P( Q - P );
   double b = 2*(v*Q_minus_P);
   double c = Q_minus_P.lengthSquared() - _radiusSquared;

   // Consider the quadratic equation
   //   t^2 + b*t + c = 0
   // If there are real roots, then the line intersects the sphere.
   // If there are *positive* roots, the the *ray* intersects the sphere.
   double determinant = b*b - 4*c;
   if ( determinant >= 0 ) {
      // see Numerical Recipes in C, pg 184
      double q = -0.5*( b+(b>0?1:-1)*sqrt(determinant) );
      double t1 = q;
      double t2 = c/q;
      if ( t1 >= 0 && t2 >= 0 ) {
         intersection = ray.point( t1 < t2 ? t1 : t2 );
         return true;
      }
      else {
         // At least one of the intersection points has a negative t value.
         // This implies that either there's no intersection between the
         // ray and sphere, or the origin of the ray is inside the sphere.
         if ( allowIntersectionEvenIfRayOriginatesInsideSphere ) {
            if ( t1 >= 0 ) {
               intersection = ray.point( t1 );
               return true;
            }
            else if ( t2 >= 0 ) {
               intersection = ray.point( t2 );
               return true;
            }
         }
      }
   }
   return false;
}


Vector3 operator*( const Matrix& m, const Vector3& v ) {
   return Vector3(
      m( 0)*v.x() + m( 4)*v.y() + m( 8)*v.z() + m(12)*v.w(),
      m( 1)*v.x() + m( 5)*v.y() + m( 9)*v.z() + m(13)*v.w(),
      m( 2)*v.x() + m( 6)*v.y() + m(10)*v.z() + m(14)*v.w(),
      m( 3)*v.x() + m( 7)*v.y() + m(11)*v.z() + m(15)*v.w()
   );
}

Matrix operator*( const Matrix& a, const Matrix& b ) {
   Matrix m;

   m( 0) = a( 0)*b( 0) + a( 4)*b( 1) + a( 8)*b( 2) + a(12)*b( 3);
   m( 1) = a( 1)*b( 0) + a( 5)*b( 1) + a( 9)*b( 2) + a(13)*b( 3);
   m( 2) = a( 2)*b( 0) + a( 6)*b( 1) + a(10)*b( 2) + a(14)*b( 3);
   m( 3) = a( 3)*b( 0) + a( 7)*b( 1) + a(11)*b( 2) + a(15)*b( 3);

   m( 4) = a( 0)*b( 4) + a( 4)*b( 5) + a( 8)*b( 6) + a(12)*b( 7);
   m( 5) = a( 1)*b( 4) + a( 5)*b( 5) + a( 9)*b( 6) + a(13)*b( 7);
   m( 6) = a( 2)*b( 4) + a( 6)*b( 5) + a(10)*b( 6) + a(14)*b( 7);
   m( 7) = a( 3)*b( 4) + a( 7)*b( 5) + a(11)*b( 6) + a(15)*b( 7);

   m( 8) = a( 0)*b( 8) + a( 4)*b( 9) + a( 8)*b(10) + a(12)*b(11);
   m( 9) = a( 1)*b( 8) + a( 5)*b( 9) + a( 9)*b(10) + a(13)*b(11);
   m(10) = a( 2)*b( 8) + a( 6)*b( 9) + a(10)*b(10) + a(14)*b(11);
   m(11) = a( 3)*b( 8) + a( 7)*b( 9) + a(11)*b(10) + a(15)*b(11);

   m(12) = a( 0)*b(12) + a( 4)*b(13) + a( 8)*b(14) + a(12)*b(15);
   m(13) = a( 1)*b(12) + a( 5)*b(13) + a( 9)*b(14) + a(13)*b(15);
   m(14) = a( 2)*b(12) + a( 6)*b(13) + a(10)*b(14) + a(14)*b(15);
   m(15) = a( 3)*b(12) + a( 7)*b(13) + a(11)*b(14) + a(15)*b(15);

   return m;
}


float LineSegment::distanceToPointSquared( int P_x, int P_y ) const {
   /*
      Consider the line B+t(A-B) through points A and B, where t is real.
      Let R be the point on the line closest to a given point P.
      We have

         (R-P)*(A-B) = 0

      Substituting in R=B+t(A-B),

         ( t(A-B) + B-P )*(A-B) = 0

         t(A-B)*(A-B) + (B-P)*(A-B) = 0

         t = (P-B)*(A-B) / |A-B|^2

      If 0 <= t <= 1, then R lies between A and B.
      Equivalently, if 0 <= (P-B)*(A-B) <= |A-B|^2,
      then R lies between A and B.

      Now, the distance squared between P and the line is

         |P-R|^2 = (P-R)*(P-R)
                 = ( P-B - t(A-B) )*( P-B - t(A-B) )
                 = (P-B)*(P-B) + t^2(A-B)*(A-B) - 2t(P-B)*(A-B)
                 = |P-B|^2 + t^2|A-B|^2 - 2t(P-B)*(A-B)

      But (P-B)*(A-B) = t|A-B|^2.  Substituting in,

         |P-R|^2 = |P-B|^2 + t(P-B)*(A-B) - 2t(P-B)*(A-B)
                 = |P-B|^2 - t(P-B)*(A-B)

      and t = (P-B)*(A-B) / |A-B|^2, so

         |P-R|^2 = |P-B|^2 - ((P-B)*(A-B))^2/|A-B|^2
   */
   int A_minus_B_x = A_x - B_x;
   int A_minus_B_y = A_y - B_y;
   float A_minus_B_length2
      = A_minus_B_x * A_minus_B_x + A_minus_B_y * A_minus_B_y;
   int P_minus_B_x = P_x - B_x;
   int P_minus_B_y = P_y - B_y;
   float dot = A_minus_B_x * P_minus_B_x + A_minus_B_y * P_minus_B_y;
   float distance;
   if ( dot <= 0 ) {
      // return the distance squared from P to B
      distance = P_minus_B_x * P_minus_B_x + P_minus_B_y * P_minus_B_y;
   }
   else if ( dot >= A_minus_B_length2 ) {
      // return the distance squared from P to A
      int P_minus_A_x = P_x - A_x;
      int P_minus_A_y = P_y - A_y;
      distance = P_minus_A_x * P_minus_A_x + P_minus_A_y * P_minus_A_y;
   }
   else {
      ASSERT( A_minus_B_length2 > 0 );
      // return the distance squared from P to the line

#if 0 // This is correct, but ...
      distance = P_minus_B_x * P_minus_B_x + P_minus_B_y * P_minus_B_y
         - dot*dot/A_minus_B_length2;
#else // ... this is slightly more efficient (and yet equivalent).
      float numerator = P_minus_B_y * A_minus_B_x - P_minus_B_x * A_minus_B_y;
      distance = numerator * numerator / A_minus_B_length2;
#endif
   }
   return distance;
}

float LineSegment::scaledComponent( int dx, int dy ) const {

   if ( dx == 0 && dy == 0 ) return 0;

   // interpret the line segment as a vector
   int dx2 = B_x - A_x;
   int dy2 = B_y - A_y;
   if ( dx2 == 0 && dy2 == 0 ) return 0;

   // compute the dot product of the vectors
   float d = dx * dx2 + dy * dy2;

   // divide by the length squared of the line segment
   float lengthSquared2 = dx2 * dx2 + dy2 * dy2;
   return d / lengthSquared2;
}


