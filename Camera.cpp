
#include "Camera.h"
#ifdef _WIN32
#include <GL/glut.h>
#else
#include <GL/gl.h>
#endif


// A good angle for the field-of-view is 30 degrees.
// Setting this very small will give the user the impression
// of looking through a telescope.
// Setting it very big will give the impression of a wide-angle
// lens, with strong foreshortening effects that are especially
// noticeable near the edges of the viewport.
const float Camera::_initialFieldOfViewSemiAngle = M_PI * 15 / 180.0;

// This should be a little greater than 1.0.
// If it's set to one, the user will initially
// just *barely* see all of the scene in the window
// (assuming the scene is a sphere of radius scene_radius).
const float Camera::_fudgeFactor = 1.1f;

// This should be a little greater than 0.0.
// If no minimum feature size is given by the user,
//    minimum_feature_size = _nearPlaneFactor * scene_radius
const float Camera::_nearPlaneFactor = 0.1f;

// far_plane = _farPlaneFactor * scene_radius
const float Camera::_farPlaneFactor = 15;

// Used for yaw, pitch, roll.
const float Camera::_rotationSpeedInDegreesPerRadius = 150;

// Used for orbit.
const float Camera::_orbitingSpeedInDegreesPerRadius = 300;

// The target (or point-of-interest) of the camera is kept
// at a minimal distance of
//   _pushThreshold * near_plane
// from the camera position.
// This number should be greater than 1.0, to ensure
// that the target is never clipped.
const float Camera::_pushThreshold = 1.3f;


void Camera::initialize(
   const Point3& scene_centre, float scene_radius,
   int window_width_in_pixels, int window_height_in_pixels,
   int viewport_x1, int viewport_y1, int viewport_x2, int viewport_y2
) {
   _initialGround = Vector3( 0, 1, 0 );
   ASSERT_IS_NORMALIZED( _initialGround );

   setSceneCentre( scene_centre );
   setSceneRadius( scene_radius );
   _minimum_feature_size = _nearPlaneFactor * _scene_radius;
   _near_plane = _minimum_feature_size;

   _window_width_in_pixels = window_width_in_pixels;
   _window_height_in_pixels = window_height_in_pixels;
   _viewport_x1 = viewport_x1;
   _viewport_y1 = viewport_y1;
   _viewport_x2 = viewport_x2;
   _viewport_y2 = viewport_y2;

   reset();
   resizeViewport(
      window_width_in_pixels, window_height_in_pixels,
      viewport_x1, viewport_y1, viewport_x2, viewport_y2
   );
}

void Camera::computeViewportWidthAndHeight() {

   // recompute _viewport_width, _viewport_height
   //
   int viewport_width_in_pixels = _viewport_x2 - _viewport_x1 + 1;
   int viewport_height_in_pixels = _viewport_y2 - _viewport_y1 + 1;
   if ( viewport_width_in_pixels < viewport_height_in_pixels ) {
      _viewport_width = 2.0 * _viewport_radius;
      _viewport_height = _viewport_width
#ifdef OPENGL_CENTRE_BASED
         * (viewport_height_in_pixels-1) / (float)(viewport_width_in_pixels-1);
#else
         * viewport_height_in_pixels / (float)viewport_width_in_pixels;
#endif
   }
   else {
      _viewport_height = 2.0 * _viewport_radius;
      _viewport_width = _viewport_height
#ifdef OPENGL_CENTRE_BASED
         * (viewport_width_in_pixels-1) / (float)(viewport_height_in_pixels-1);
#else
         * viewport_width_in_pixels / (float)viewport_height_in_pixels;
#endif
   }
}

void Camera::setSceneRadius( float r ) {
   ASSERT( r > 0 );
   if ( r <= 0 ) return;
   _scene_radius = r;
   _far_plane = _farPlaneFactor * _scene_radius;
}

void Camera::setMinimumFeatureSize( float s ) {
   ASSERT( s > 0 );
   if ( s <= 0 ) return;
   _minimum_feature_size = s;

   ASSERT( _near_plane > 0 );
   float k = _minimum_feature_size / _near_plane;
   _near_plane = _minimum_feature_size;
   _viewport_radius *= k;
   computeViewportWidthAndHeight();
}

void Camera::reset() {

   double tangent = tan( _initialFieldOfViewSemiAngle );

   _viewport_radius = _near_plane * tangent;
   float distance_from_target = _fudgeFactor * _scene_radius / tangent;

   computeViewportWidthAndHeight();

   // The ground vector can change if the user rolls the camera,
   // hence we reset it here.
   _ground = _initialGround;

   _up = _ground;

   Vector3 v;
   if ( _up.x()==0 && _up.y()==1 && _up.z()==0 )
      v = Vector3( 0, 0, 1 );
   else v = _up.choosePerpendicular().normalized();

   _target = _scene_centre;
   _position = _target + v * distance_from_target;
}

void Camera::resizeViewport(
   int window_width_in_pixels, int window_height_in_pixels,
   int viewport_x1, int viewport_y1,
   int viewport_x2, int viewport_y2
) {
   ASSERT( viewport_x1 < viewport_x2 );
   ASSERT( viewport_y1 < viewport_y2 );

   _window_width_in_pixels = window_width_in_pixels;
   _window_height_in_pixels = window_height_in_pixels;
   _viewport_x1 = viewport_x1;
   _viewport_y1 = viewport_y1;
   _viewport_x2 = viewport_x2;
   _viewport_y2 = viewport_y2;
   _viewport_cx = ( _viewport_x1 + _viewport_x2 ) * 0.5;
   _viewport_cy = ( _viewport_y1 + _viewport_y2 ) * 0.5;

#ifdef OPENGL_CENTRE_BASED
   float viewport_radius_x = _viewport_cx - _viewport_x1;
   float viewport_radius_y = _viewport_cy - _viewport_y1;
#else
   float viewport_radius_x = _viewport_cx - _viewport_x1 + 0.5f;
   float viewport_radius_y = _viewport_cy - _viewport_y1 + 0.5f;
#endif
   _viewport_radius_in_pixels = ( viewport_radius_x < viewport_radius_y )
      ? viewport_radius_x
      : viewport_radius_y;

   computeViewportWidthAndHeight();

   // This function expects coordinates relative to an origin
   // in the lower left corner of the window (i.e. y+ is up).
#ifdef OPENGL_CENTRE_BASED
   // Also, the dimensions passed in should be between pixel centres.
#else
   // The coordinates are edge-based.
   // Also, the dimensions passed in should be between pixel edges.
#endif
   //
   // Say you have a window composed of the pixels marked with '.',
   // and want a viewport over the pixels marked with 'x':
   //
   //    ............
   //    ............
   //    .xxxxxxx....
   //    .xxxxxxx....
   //    .xxxxxxx....
   //    .xxxxxxx....
   //    .xxxxxxx....
   //    ............
   //    ............
   //    ............
   //
#ifdef OPENGL_CENTRE_BASED
   // You would have to call glViewport( 1, 3, 6, 4 );
#else
   // You would have to call glViewport( 1, 3, 7, 5 );
#endif
   //
   glViewport(
      _viewport_x1,
      _window_height_in_pixels - 1 - _viewport_y2,
#ifdef OPENGL_CENTRE_BASED
      _viewport_x2 - _viewport_x1,
      _viewport_y2 - _viewport_y1
#else
      _viewport_x2 - _viewport_x1 + 1,
      _viewport_y2 - _viewport_y1 + 1
#endif
   );
}

void Camera::transform() {

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();  // clear matrix

   ASSERT( _near_plane < _far_plane );

   glFrustum(
      - 0.5 * _viewport_width,  0.5 * _viewport_width,    // left, right
      - 0.5 * _viewport_height, 0.5 * _viewport_height,   // bottom, top
      _near_plane, _far_plane
   );

   Matrix m;
   m.setToLookAt( _position, _target, _up );
   glMultMatrixf( m.get() );
}

void Camera::pushViewportAndTransform(
   int window_width_in_pixels, int window_height_in_pixels,
   int viewport_x1, int viewport_y1,
   int viewport_x2, int viewport_y2,
   int originalViewport[4]
) {
   glGetIntegerv( GL_VIEWPORT, originalViewport );
   resizeViewport(
      window_width_in_pixels, window_height_in_pixels,
      viewport_x1, viewport_y1,
      viewport_x2, viewport_y2
   );
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   transform();
}

void Camera::popViewportAndTransform( int originalViewport[4] ) {
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();
   glMatrixMode( GL_MODELVIEW );
   glViewport(
      originalViewport[0], originalViewport[1],
      originalViewport[2], originalViewport[3]
   );
}

void Camera::zoomIn( float delta_pixels ) {

   // this should be a little greater than 1.0
   static const float magnificationFactorPerPixel = 1.005;

   _viewport_radius *= pow( magnificationFactorPerPixel, - delta_pixels );

   computeViewportWidthAndHeight();
}

void Camera::orbit(
   float old_x_pixels, float old_y_pixels,
   float new_x_pixels, float new_y_pixels
) {
   float pixelsPerDegree = _viewport_radius_in_pixels
      / _orbitingSpeedInDegreesPerRadius;
   float radiansPerPixel = 1.0 / pixelsPerDegree * M_PI / 180;

   Vector3 t2p = _position - _target;

   Matrix m;
   m.setToRotation(
      (old_x_pixels-new_x_pixels) * radiansPerPixel,
      _ground
   );
   t2p = m*t2p;
   _up = m*_up;
   Vector3 right = (_up ^ t2p).normalized();
   m.setToRotation(
      (old_y_pixels-new_y_pixels) * radiansPerPixel,
      right
   );
   t2p = m*t2p;
   _up = m*_up;
   _position = _target + t2p;
}

void Camera::orbit(
   float old_x_pixels, float old_y_pixels,
   float new_x_pixels, float new_y_pixels,
   const Point3& centre
) {
   float pixelsPerDegree = _viewport_radius_in_pixels
      / _orbitingSpeedInDegreesPerRadius;
   float radiansPerPixel = 1.0 / pixelsPerDegree * M_PI / 180;

   Vector3 t2p = _position - _target;
   Vector3 c2p = _position - centre;

   Matrix m;
   m.setToRotation(
      (old_x_pixels-new_x_pixels) * radiansPerPixel,
      _ground
   );
   c2p = m*c2p;
   t2p = m*t2p;
   _up = m*_up;
   Vector3 right = (_up ^ t2p).normalized();
   m.setToRotation(
      (old_y_pixels-new_y_pixels) * radiansPerPixel,
      right
   );
   c2p = m*c2p;
   t2p = m*t2p;
   _up = m*_up;
   _position = centre + c2p;
   _target = _position - t2p;
}

void Camera::translateSceneRightAndUp(
   float delta_x_pixels, float delta_y_pixels
) {
   Vector3 direction = _target - _position;
   float distance_from_target = direction.length();
   direction = direction.normalized();

   float translationSpeedInUnitsPerRadius =
      distance_from_target * _viewport_radius / _near_plane;
   float pixelsPerUnit = _viewport_radius_in_pixels
      / translationSpeedInUnitsPerRadius;

   Vector3 right = direction ^ _up;

   Vector3 translation
      = right * ( - delta_x_pixels / pixelsPerUnit )
      + _up * ( - delta_y_pixels / pixelsPerUnit );

   _position += translation;
   _target += translation;
}

void Camera::dollyCameraForward( float delta_pixels, bool pushTarget ) {
   Vector3 direction = _target - _position;
   float distance_from_target = direction.length();
   direction = direction.normalized();

   float translationSpeedInUnitsPerRadius =
      distance_from_target * _viewport_radius / _near_plane;
   float pixelsPerUnit = _viewport_radius_in_pixels
      / translationSpeedInUnitsPerRadius;

   float dollyDistance = delta_pixels / pixelsPerUnit;

   if ( ! pushTarget ) {
      distance_from_target -= dollyDistance;
      if ( distance_from_target < _pushThreshold*_near_plane ) {
         distance_from_target = _pushThreshold*_near_plane;
      }
   }

   _position += direction * dollyDistance;
   _target = _position + direction * distance_from_target;
}

void Camera::pitchCameraUp( float delta_pixels ) {

   float pixelsPerDegree = _viewport_radius_in_pixels
      / _rotationSpeedInDegreesPerRadius;
   float angle = delta_pixels / pixelsPerDegree * M_PI / 180;

   Vector3 p2t = _target - _position;

   Vector3 right = (p2t.normalized()) ^ _up;

   Matrix m;
   m.setToRotation( angle, right );
   p2t = m*p2t;
   _up = m*_up;
   _target = _position + p2t;
}

void Camera::yawCameraRight( float delta_pixels ) {

   float pixelsPerDegree = _viewport_radius_in_pixels
      / _rotationSpeedInDegreesPerRadius;
   float angle = delta_pixels / pixelsPerDegree * M_PI / 180;

   Vector3 p2t = _target - _position;

   Matrix m;
   m.setToRotation( -angle, _ground );
   p2t = m*p2t;
   _up = m*_up;
   _target = _position + p2t;
}

void Camera::rollCameraRight( float delta_pixels ) {

   float pixelsPerDegree = _viewport_radius_in_pixels
      / _rotationSpeedInDegreesPerRadius;
   float angle = delta_pixels / pixelsPerDegree * M_PI / 180;

   Vector3 direction = (_target - _position).normalized();

   Matrix m;
   m.setToRotation( angle, direction );
   _ground = m*_ground;
   _up = m*_up;
}

void Camera::lookAt( const Point3& p ) {
   // FIXME: we do not check if the target point is too close
   // to the camera (i.e. less than _pushThreshold * near_plane ).
   // If it is, perhaps we should dolly the camera away from the
   // target to maintain the minimal distance.

   _target = p;
   Vector3 direction = (_target - _position).normalized();
   Vector3 right = ( direction ^ _ground ).normalized();
   _up = right ^ direction;
   ASSERT_IS_NORMALIZED( _up );
}

void Camera::zBufferLookAt( int pixel_x, int pixel_y ) {

   int W = _viewport_x2 - _viewport_x1 + 1;
   int H = _viewport_y2 - _viewport_y1 + 1;

   GLfloat * zbuffer = new GLfloat[ W * H ];

   // FIXME: we want to fetch the z-buffer of the most
   // recently rendered image.  Unfortunately, this code
   // probably grabs the back z-buffer, rather than the
   // front z-buffer, which (if I'm not mistaken) is what we want.
   // FIXME: instead of grabbing the whole buffer, we should just
   // grab a small region around the pixel of interest.
   // Grabbing the whole buffer is very slow on
   // some Iris machines (e.g. Indigo2).
   glReadPixels(
      _viewport_x1,
      _window_height_in_pixels - 1 - _viewport_y2,
      W, H,
      GL_DEPTH_COMPONENT, GL_FLOAT, zbuffer
   );

   // // Print contents of z-buffer (for debugging only).
   // for ( int b = H-1; b >= 0; --b ) {
   //    for ( int a = 0; a < W; ++a ) {
   //       printf("%c",zbuffer[ b*W + a ]==1?'.':'X');
   //    }
   //    puts("");
   // }

   // Keep in mind that zbuffer[0] corresponds to the lower-left
   // pixel of the viewport, *not* the upper-left pixel.

   // Transform the pixel passed in so that it is
   // in the z-buffer's coordinate system.
   pixel_x -= _viewport_x1;
   pixel_y = H - 1 - (pixel_y - _viewport_y1);

   // Assume, as is typically the case, that the z-buffer was
   // cleared with a value of 1.0f (the maximum z value).
   // Also assume that any pixels containing a value of 1.0f
   // have this value because no rendered geometry covered them.
   // Now, search outward from the pixel passed in, in larger
   // and larger rectangles, for a pixel with a z value
   // less than 1.0f.

   ASSERT( 0 <= pixel_x && pixel_x < W && 0 <= pixel_y && pixel_y < H );

   // the search rectangle
   int x1 = pixel_x, y1 = pixel_y;
   int x2 = x1, y2 = y1;

   int x,y,x0=W/2,y0=H/2;
   int min, max;
   float z = 1;
   while ( z == 1 ) {
      // top edge of rectangle
      if ( y1 >= 0 ) {
         y = y1;
         min = x1 < 0 ? 0 : x1;
         max = x2 >= W ? W-1 : x2;
         for ( x = min; x <= max; ++x )
            if ( zbuffer[ y*W + x ] < z ) {
               z = zbuffer[ y*W + x ];
               x0 = x;
               y0 = y;
            }
      }
      // bottom edge of rectangle
      if ( y2 < H ) {
         y = y2;
         min = x1 < 0 ? 0 : x1;
         max = x2 >= W ? W-1 : x2;
         for ( x = min; x <= max; ++x )
            if ( zbuffer[ y*W + x ] < z ) {
               z = zbuffer[ y*W + x ];
               x0 = x;
               y0 = y;
            }
      }
      // left edge of rectangle
      if ( x1 >= 0 ) {
         x = x1;
         min = y1 < 0 ? 0 : y1;
         max = y2 >= H ? H-1 : y2;
         for ( y = min; y <= max; ++y )
            if ( zbuffer[ y*W + x ] < z ) {
               z = zbuffer[ y*W + x ];
               x0 = x;
               y0 = y;
            }
      }
      // right edge of rectangle
      if ( x2 < W ) {
         x = x2;
         min = y1 < 0 ? 0 : y1;
         max = y2 >= H ? H-1 : y2;
         for ( y = min; y <= max; ++y )
            if ( zbuffer[ y*W + x ] < z ) {
               z = zbuffer[ y*W + x ];
               x0 = x;
               y0 = y;
            }
      }

      // grow the rectangle
      --x1;  --y1;
      ++x2;  ++y2;

      // is there nothing left to search ?
      if ( y1 < 0 && y2 >= H && x1 < 0 && x2 >= W )
         break;
   }

   if ( z < 1 ) {
      // compute point in clipping space
#ifdef OPENGL_CENTRE_BASED
      Point3 pc( 2*x0/(float)(W-1)-1, 2*y0/(float)(H-1)-1, 2*z-1 );
#else
      // The +0.5f here is to convert from centre-based coordinates
      // to edge-based coordinates.
      Point3 pc( 2*(x0+0.5f)/(float)W-1, 2*(y0+0.5f)/(float)H-1, 2*z-1 );
#endif

      // compute inverse view transform
      Matrix M;
      M.setToLookAt( _position, _target, _up, true );

      // compute inverse perspective transform
      Matrix M2;
      M2.setToFrustum(
         - 0.5 * _viewport_width,  0.5 * _viewport_width,    // left, right
         - 0.5 * _viewport_height, 0.5 * _viewport_height,   // bottom, top
         _near_plane, _far_plane,
         true
      );

      // compute inverse of light transform
      M = M * M2;

      // compute point in world space
      Point3 t0 = M * pc;
      Point3 t( t0.x()/t0.w(), t0.y()/t0.w(), t0.z()/t0.w() );

      lookAt( t );
   }

   delete [] zbuffer;
   zbuffer = 0;
}

void Camera::framePoints( const vector< Point3 >& l, bool frameExactly ) {
   if ( l.empty() )
      return;
   Vector3 direction = (_target - _position).normalized();
   Vector3 right = direction ^ _up;

   // Transform each of the points to camera space,
   // and find the minimal bounding box containing them
   // that is aligned with camera space.
   // FIXME the projection we do here is orthogonal;
   // it ignores the perspective projection of the camera
   // that will make distant points appear closer to the line of sight.
   // Thus, the camera will not frame points as tightly as it could.
   AlignedBox box;
   vector< Point3 >::const_iterator it = l.begin();
   for ( ; it != l.end(); ++it ) {
      Vector3 v = (*it) - _position;
      box.bound( Point3( v * right, v * _up, v * direction ) );
   }

   // Translate the camera such
   // that the line of sight passes through the centre of the bounding box,
   // and the target point is at the centre of the bounding box.
   Point3 boxCentre = box.getCentre();
   Point3 newTarget = _position
      + right * boxCentre.x() + _up * boxCentre.y() + direction * boxCentre.z();
   Vector3 translation = newTarget - _target;
   _position += translation;
   _target = newTarget;

   // Next, dolly the camera forward (or backward)
   // such that all points are visible.
   // For each given point,
   // we compute the amount by which to dolly forward for that point,
   // and find the minimum of all such amounts
   // to use for the actual dolly.
   /*
      In the below diagram,
         "1" is the current position of the camera, which is pointing down
         "a" is the distance to the near plane
         "b" is half the width/height of the viewport
         "3" is a point we want to frame by dollying the camera forward
         "2" is the new location to place the camera to just frame "3"
         "?" is the distance to dolly forward, which we want to compute

                   |-b-|

                       1  -  -  -
                      /|  |  |  |
                     / |  a  |  |
                    /  |  |  |  |
                   /---|  -  ?  |
                  /    |     |  |
                 /     |     |  |
                /      |     |  c
               /       2     -  |
              /       /|     |  |
             /       / |     |  |
            /       /  |     e  |
           /       /   |     |  |
          /       /    |     |  |
         .-------3-----.     -  -

                 |--f--|

         |------d------|

      The two hypotenuses are parallel because we don't want to change
      the camera's field of view.
      We have
         a/b = c/d = e/f   (where a,b,c,f are known)
      and
         ? = c - e
           = c - f*a/b
   */

   float minDollyDelta = 0;
   bool minDollyDeltaInitialized = false;
   float min_c = 0;
   for ( it = l.begin(); it != l.end(); ++it ) {
      Vector3 v = (*it) - _position;
      float c = v * direction;

      if ( it == l.begin() ) min_c = c;
      else if ( c < min_c ) min_c = c;

      float f, dollyDelta;
      if ( _viewport_width > 0 ) {
         f = fabs( v * right );
         dollyDelta = c - f * _near_plane * 2 / _viewport_width;
         if ( minDollyDeltaInitialized ) {
            if ( dollyDelta < minDollyDelta )
               minDollyDelta = dollyDelta;
         }
         else {
            minDollyDelta = dollyDelta;
            minDollyDeltaInitialized = true;
         }
      }
      if ( _viewport_height > 0 ) {
         f = fabs( v * _up );
         dollyDelta = c - f * _near_plane * 2 / _viewport_height;
         if ( minDollyDeltaInitialized ) {
            if ( dollyDelta < minDollyDelta )
               minDollyDelta = dollyDelta;
         }
         else {
            minDollyDelta = dollyDelta;
            minDollyDeltaInitialized = true;
         }
      }
   }

   if ( ! frameExactly ) {
      // check that the dolly won't put some points
      // in front of the near plane
      if ( min_c - minDollyDelta < _near_plane )
         minDollyDelta = min_c - _near_plane;

      // fudge factor, to add a tiny margin around the points
      minDollyDelta -= 0.05f * _near_plane;
   }

   // perform the dolly
   _position += direction * minDollyDelta;

   if ( ( _target - _position ).length() <= _pushThreshold*_near_plane ) {
      // The target point is too close.
      _target = _position + direction * ( _pushThreshold*_near_plane );
   }

   // FIXME: should we check if some points are beyond the
   // far plane, and if so move the far plane further back ?
}

void Camera::frameBox( const AlignedBox& box ) {
   vector< Point3 > corners;
   for ( int i = 0; i < 8; ++i )
      corners.push_back( box.getCorner( i ) );
   framePoints( corners );
}

Ray Camera::computeRay( int pixel_x, int pixel_y ) const {
   // this is a point on the near plane, in camera space
   Point3 p(
      (pixel_x-_viewport_cx)*_viewport_radius/_viewport_radius_in_pixels,
      (_viewport_cy-pixel_y)*_viewport_radius/_viewport_radius_in_pixels,
      _near_plane
   );

   // transform p to world space
   Vector3 direction = (_target - _position).normalized();
   Vector3 right = direction ^ _up;
   Vector3 v = right*p.x() + _up*p.y() + direction*p.z();
   p = _position + v;

   return Ray( p, v.normalized() );
}

float Camera::computePixel(
   const Point3& p, int& pixel_x, int& pixel_y
) const {
   // Transform the point from world space to camera space.

   Vector3 direction = (_target - _position).normalized();
   Vector3 right = direction ^ _up;

   // Note that (right, _up, direction) form an orthonormal basis.
   // To transform a point from camera space to world space,
   // we can use the 3x3 matrix formed by concatenating the
   // 3 vectors written as column vectors.  The inverse of such
   // a matrix is simply its transpose.  So here, to convert from
   // world space to camera space, we do

   Vector3 v = p-_position;
   float x = v*right;
   float y = v*_up;
   float z = v*direction;

   // (or, more simply, the projection of a vector onto a unit vector
   // is their dot product)

   float k = _near_plane / z;

   // The +0.5f here is for rounding.
   pixel_x = (int)(
      k*_viewport_radius_in_pixels*x/_viewport_radius + _viewport_cx + 0.5f
   );
   pixel_y = (int)(
      _viewport_cy - k*_viewport_radius_in_pixels*y/_viewport_radius + 0.5f
   );
   return z;
}

float Camera::convertLength(
   float z_distance, float fractionOfViewportSize
) const {
   return z_distance * fractionOfViewportSize * 2
      * _viewport_radius / _near_plane; //tangent of field-of-view's semi-angle
}

void Camera::interpolate( const Camera& c1, const Camera& c2, float u ) {
   // FIXME I only bother to interpolate a few data members.
   // You may need to add more data members here eventually.
   _position = Point3( Vector3(c1._position)*(1-u) + Vector3(c2._position)*u );
   _target = Point3( Vector3(c1._target)*(1-u) + Vector3(c2._target)*u );
   _up = c1._up*(1-u) + c2._up*u;
}

void Camera::interpolateThroughOverview(
   const Camera& c1, const Camera& c2, float u
) {
   // initialize our state with the proper orientation, field of view, etc.
   *this = c1;

   // set our state equal to the intermediate, "overview" state
   vector< Point3 > l;
   l.push_back( c1.getPosition() );
   l.push_back( c2.getPosition() );
   framePoints( l, true );

   // compute the value of u corresponding to the "overview" state
   float length1 = ( _position - c1._position ).length();
   float length2 = ( _position - c2._position ).length();
   if ( length1 + length2 == 0 ) {
      // For definiteness (e.g. to ensure our state has
      // the correct target point and other parameters)
      // set our state equal to one of the given states.
      *this = u < 0.5f ? c1 : c2;
      return;
   }
   float u_overview = length1 / ( length1 + length2 );

   // now interpolate linearly between our state and one of the end states
   if ( u < u_overview ) {
      u /= u_overview; // normalize u
      _position = Point3( Vector3(c1._position)*(1-u) + Vector3(_position)*u );
      _target = Point3( Vector3(c1._target)*(1-u) + Vector3(_target)*u );
      _up = c1._up*(1-u) + _up*u;
   }
   else {
      u = (u-u_overview)/(1-u_overview); // normalize u
      _position = Point3( Vector3(_position)*(1-u) + Vector3(c2._position)*u );
      _target = Point3( Vector3(_target)*(1-u) + Vector3(c2._target)*u );
      _up = _up*(1-u) + c2._up*u;
   }
}
