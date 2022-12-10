
#ifndef CAMERA_H
#define CAMERA_H


#include "mathutil.h"


class Camera {

   // Definitions:
   //
   // A dimension (e.g. width, height, size, length) expressed in pixel
   // units can be MEASURED BETWEEN PIXEL CENTRES or
   // MEASURED BETWEEN PIXEL EDGES.
   // Consider, for example, a white filled-in square displayed
   // in the centre of an otherwise black screen.
   // Assume that, when rasterized, the square causes 10x10==100
   // physical pixels to light up.  In this case, the width and
   // height of the square are 10 when measured between pixel edges,
   // and the width and height of the square are 9 when measured
   // between pixel centres.
   //
   // The coordinates of a point expressed in pixel units are
   // measured with respect to an origin.  For our purposes,
   // the origin can be assumed to be located at a corner
   // of the viewport (typically the upper-left or
   // lower-left corner).  Call this corner the extremal corner,
   // and call the pixel in the viewport that is closest to
   // the extremal corner the extremal pixel.
   // The origin may coincide with the
   // centre of the extremal pixel, or with the (extremal) corner
   // of the extremal pixel.  In the former case, we will say
   // the coordinates are CENTRE-BASED; in the latter, we will
   // say the coordinates are EDGE-BASED.
   // Typically, with centre-based coordinates, the axes of the coordinate
   // system pass through the centres of the top (or bottom)
   // row of pixels, and through the centres of the left-most
   // column of pixels.  With edge-based coordinates, the
   // axes lie along pixel edges, typically the outer edges
   // of the top (or bottom) row of pixels,
   // and the left edges of the left-most column of pixels.
   // With centre-based coordinates, the coordinates of the
   // centre of the extremal pixel are (0,0).  With edge-based
   // coordinates, the coordinates of the centre of the extremal
   // pixel are (0.5,0.5).
   //
   // So, in summary, pixel dimensions will be either measured
   // between pixel centres or between pixel edges.
   // Pixel coordinates will either be centre-based or edge-based.
   //
   // In the API for this class, unless otherwise specified,
   // dimensions are measured between pixel edges,
   // and pixel coordinates are centre-based.

// This is almost certainly not true
// (see OpenGL Programming Manual, by Woo, Neider, Davis and Shreiner;
// Appendix H, the note on "exact two-dimensional rasterization"
// on page 313 (?))
// (Also, I found this in the man page for glScissor:
// "Window coordinates have integer values at
// the shared corners of frame buffer pixels.")
//#define OPENGL_CENTRE_BASED

private:
   // window dimensions (measured between pixel edges)
   // (the only reason we need these is to convert client coordinates,
   // which are relative to the upper-left corner, to coordinates for
   // OpenGL, which are relative to the lower-left corner)
   int _window_width_in_pixels, _window_height_in_pixels;

   // pixel coordinates (centre-based) of the viewport's corners
   int _viewport_x1, _viewport_y1, _viewport_x2, _viewport_y2;

   // pixel coordinates (centre-based) of the viewport's centre.
   // These will be half-integers if the viewport's dimensions are even.
   float _viewport_cx, _viewport_cy;

   // the radius of the viewport, in pixels
#ifdef OPENGL_CENTRE_BASED
   // (measured between pixel centres)
#else
   // (measured between pixel edges)
#endif
   float _viewport_radius_in_pixels;

   // dimensions of the viewport, in world space, measured on the near plane
   float _viewport_radius, _viewport_width, _viewport_height;

   // camera state
   Point3 _scene_centre;
   float _scene_radius, _minimum_feature_size;
   float _near_plane, _far_plane; // clipping planes
   Point3 _position; // point of view
   Point3 _target;   // point of interest
   Vector3 _up;      // a unit vector, perpendicular to the line-of-sight
   Vector3 _ground;  // a unit vector perpendicular to the ground plane
   Vector3 _initialGround;

   // constants that can be tuned
   static const float _initialFieldOfViewSemiAngle;
   static const float _fudgeFactor;
   static const float _nearPlaneFactor;
   static const float _farPlaneFactor;
   static const float _rotationSpeedInDegreesPerRadius; // for yaw, pitch, roll
   static const float _orbitingSpeedInDegreesPerRadius; // for orbit
   static const float _pushThreshold;

   void initialize(
      const Point3& scene_centre, float scene_radius,
      int window_width_in_pixels, int window_height_in_pixels,
      int viewport_x1, int viewport_y1, int viewport_x2, int viewport_y2
   );
   void computeViewportWidthAndHeight();

public:

   Camera(
      int window_width_in_pixels,  // measured between pixel edges
      int window_height_in_pixels, // measured between pixel edges
      float scene_radius,
      const Point3& scene_centre = Point3(0,0,0)
   ) {
      initialize(
         scene_centre, scene_radius,
         window_width_in_pixels, window_height_in_pixels,
         0, 0, window_width_in_pixels - 1, window_height_in_pixels - 1
      );
   }

   Camera(
      int window_width_in_pixels,  // measured between pixel edges
      int window_height_in_pixels, // measured between pixel edges
      int viewport_x1, int viewport_y1, // centre-based coordinates
      int viewport_x2, int viewport_y2, // centre-based coordinates
      float scene_radius,
      const Point3& scene_centre = Point3(0,0,0)
   ) {
      initialize(
         scene_centre, scene_radius,
         window_width_in_pixels, window_height_in_pixels,
         viewport_x1, viewport_y1, viewport_x2, viewport_y2
      );
   }

   void setSceneCentre( const Point3& p ) {
      _scene_centre = p;
   }

   // This causes the far plane to be recomputed.
   void setSceneRadius( float );

   // This causes the near plane to be recomputed.
   // Note that the argument passed in must be positive.
   // Also note that smaller arguments will pull the
   // near plane closer, but will also reduce the
   // z-buffer's precision.
   void setMinimumFeatureSize( float );

   void reset();

   void resizeViewport(
      int window_width_in_pixels, // measured between pixel edges
      int window_height_in_pixels // measured between pixel edges
   ) {
      resizeViewport(
         window_width_in_pixels, window_height_in_pixels,
         0, 0, window_width_in_pixels - 1, window_height_in_pixels - 1
      );
   }

   void resizeViewport(
      int window_width_in_pixels,  // measured between pixel edges
      int window_height_in_pixels, // measured between pixel edges
      int viewport_x1, int viewport_y1, // centre-based coordinates
      int viewport_x2, int viewport_y2  // centre-based coordinates
   );

   void transform();

   // Clients that want to, for example, use this class to draw
   // 3D graphics within a smaller viewport on top of a larger
   // 2D graphical scene (such as surrounding widgets),
   // can use these methods to prepare for and clean up after,
   // respectively, drawing 3D stuff.
   void pushViewportAndTransform(
      int window_width_in_pixels, int window_height_in_pixels,
      int viewport_x1, int viewport_y1,
      int viewport_x2, int viewport_y2,

      // Original viewport is saved here,
      // and must be stored by the client for popping later.
      int originalViewport[4]
   );
   void popViewportAndTransform( int originalViewport[4] );

   const Point3& getPosition() const { return _position; }
   const Point3& getTarget() const { return _target; }
   const Vector3& getUp() const { return _up; }
   Vector3 getRight() const {
      Vector3 direction = ( _target - _position ).normalized();
      return direction ^ _up;
   }

   int getViewportWidthInPixels() const { return _window_width_in_pixels; }
   int getViewportHeightInPixels() const { return _window_height_in_pixels; }

   // Changes the field-of-view.
   void zoomIn( float delta_pixels );

   // These "orbit" the camera, causing the scene to "tumble".
   // Normally, orbiting is done around the camera's target.
   // However, if the client supplies a centre point, orbiting
   // will be done around the given centre, and the camera target
   // will be adjusted accordingly.
   void orbit(
      float old_x_pixels, float old_y_pixels,
      float new_x_pixels, float new_y_pixels
   );
   void orbit(
      float old_x_pixels, float old_y_pixels,
      float new_x_pixels, float new_y_pixels,
      const Point3& centre
   );
   void orbit( float delta_x_pixels, float delta_y_pixels ) {
      orbit(
         _viewport_cx, _viewport_cy,
         _viewport_cx + delta_x_pixels, _viewport_cy + delta_y_pixels
      );
   }
   void orbit(
      float delta_x_pixels, float delta_y_pixels,
      const Point3& centre
   ) {
      orbit(
         _viewport_cx, _viewport_cy,
         _viewport_cx + delta_x_pixels, _viewport_cy + delta_y_pixels,
         centre
      );
   }

   // Translates the point-of-interest in the camera plane.
   // Also translates the camera by a same amount.
   // Effectively *moves* the camera up/down/right/left,
   // without changing its orientation.
   // Some may refer to this as "panning" the camera.
   void translateSceneRightAndUp( float delta_x_pixels, float delta_y_pixels );

   // Moves the camera forward or backward.  Some refer to
   // this as "tracking" the camera.
   // If the boolean flag is true, the target (or point of interest)
   // is moved with the camera, keeping the distance between the two
   // constant.
   void dollyCameraForward( float delta_pixels, bool pushTarget );

   // Changes the elevation angle of the camera.
   // Some refer to this as "tilting" the camera.
   void pitchCameraUp( float delta_pixels );

   // Changes the azimuth angle of the camera.
   // Some refer to this as "panning" the camera.
   void yawCameraRight( float delta_pixels );

   // Rolls the camera.
   void rollCameraRight( float delta_pixels );

   // Rotates the camera toward the given point.
   void lookAt( const Point3& );

   // Uses the z-buffer to find the nearest rendered pixel,
   // and makes the camera look at it.
   void zBufferLookAt( int pixel_x, int pixel_y );

   // Unlinke lookAt(), these methods do NOT change the camera's orientation.
   // Instead, they translate the camera
   // such that the given points (or corners of the given box)
   // are just visible within the viewport.
   void framePoints( const vector< Point3 >&, bool frameExactly = false );
   void frameBox( const AlignedBox& );

   // Returns the ray through the centre of the given pixel.
   Ray computeRay(
      int pixel_x, int pixel_y   // centre-based coordinates
   ) const;
   // Computes the pixel covering the given point.
   // Also returns the z-distance (in camera space) to the point.
   float computePixel(
      const Point3&,
      int& pixel_x, int& pixel_y   // centre-based coordinates
   ) const;

   // Compute the necessary size, in world space, of an object
   // (at the given distance from the camera,
   // or centred at the given point, respectively)
   // for it to cover the given fraction of the viewport.
   // These methods are useful for determining the necessary size of widgets
   // placed in 3D space for them to have a constant size in screen space.
   float convertLength( float z_distance, float fractionOfViewportSize ) const;
   float convertLength( const Point3& p, float fractionOfViewportSize ) const {
      int x, y;
      return convertLength( computePixel( p, x, y ), fractionOfViewportSize );
   }

   // Compute the necessary size, in world space, of an object
   // centred at the given point
   // for it to cover the given length of pixels.
   float convertPixelLength( const Point3& p, int pixelLength ) const {
      int x, y;
      return convertLength(
         computePixel( p, x, y ),
         0.5 * pixelLength / (float)_viewport_radius_in_pixels
      );
   }

   // Sets the state of the camera to be an
   // interpolation of the states of the given cameras.
   // The interpolation parameter should be in the interval [0,1]
   // This method can be used for animating camera motions.
   void interpolate( const Camera& c1, const Camera& c2, float u );

   // Similar to interpolate(),
   // but rather than performing a linear interpolation,
   // this routine interpolates through an
   // "overview" of what both c1 and c2 see.
   // Thus, when this routine is used for animating camera transitions
   // from one place to another (possibly distant) place,
   // at some point during the transition
   // the user can see both places simultaneously,
   // and thus gain a better idea of how they are related spatially.
   // As currently implemented, this routine assumes that
   // c1 and c2 have the same orientation and the same field-of-view.
   void interpolateThroughOverview(
      const Camera& c1, const Camera& c2, float u
   );
};


#endif /* CAMERA_H */

