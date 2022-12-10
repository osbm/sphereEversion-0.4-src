
#ifndef DRAWUTIL_H
#define DRAWUTIL_H


#include "mathutil.h"


void drawBox(
   const AlignedBox& box, bool expand, bool makeItLookCool
);
void drawCircle(
   const Point3& centre,
   const Vector3& normal, // a unit vector normal to the circle's plane
   float radius,
   float arcLengthPerPixel,
   bool isDashed = false
);
void drawShadedCircle(
   const Point3& centre,
   const Vector3& normal, // a unit vector normal to the circle's plane
   float radius,
   float arcLengthPerPixel,

   // The circle that is drawn is (loosely speaking) "shaded" to give the user
   // more depth cues.  Specifically, the thickness and colour
   // of the circle are varied to emphasize the part of the circle
   // that is nearest to the user.
   // The calling code provides values of thickness and colour
   // for portions of the circle that are far or near to the view point,
   // and the function interpolates as appropriate.
   float farThickness,           // in pixels
   float nearThickness,          // in pixels
   const Point3& farColour,      // interpreted as an rgb triple
   const Point3& nearColour,     // interpreted as an rgb triple
   const Point3& cameraLocation, // the "eye" point

   bool isDashed = false,

   bool isArc = false,
   // This vector should be perpendicular to the circle's normal.
   const Vector3 & radialVectorAtWhichArcStarts = Vector3(1,0,0),
   float arcAngle = 0            // in radians; should be in [0,2*pi]
);
void drawCone(
   const Point3& apex,
   const Vector3& axis, // a unit vector pointing from apex to base
   float semiAngle,
   float lateralLength,
   int numLateralSides,
   bool drawBase
);
void drawFrame(
   const Point3& origin,
   const Vector3& i, const Vector3& j, const Vector3& k,//must be unit vectors
   float length, bool drawArrowHeads, bool isColoured,
   const Point3& i_colour = Point3(1,0,0), // interpreted as an rgb triple
   const Point3& j_colour = Point3(0,1,0), // interpreted as an rgb triple
   const Point3& k_colour = Point3(0,0,1)  // interpreted as an rgb triple
);
inline void drawAxes( const Point3& origin, float length, bool isColoured ) {
   drawFrame(
      origin, Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1),
      length, true, isColoured
   );
}
void drawCrossHairs( const Point3& centre, float radius );

void drawRay( const Ray& ray, float length, float arrowHeadLength );
inline void drawRay( const Ray& ray, float length = 5 ) {
   drawRay( ray, length, length/15 );
}
void drawShadedRay(
   const Ray& ray, float length, float arrowHeadLength,

   // The ray that is drawn is (loosely speaking) "shaded" to give the user
   // more depth cues.  Specifically, the thickness and colour
   // of the ray are varied to emphasize the part of the circle
   // that is nearest to the user.
   // The calling code provides values of thickness and colour
   // for ends of the ray that are far or near to the view point,
   // and the function interpolates as appropriate.
   float farThickness,           // in pixels
   float nearThickness,          // in pixels
   const Point3& farColour,      // interpreted as an rgb triple
   const Point3& nearColour,     // interpreted as an rgb triple
   const Point3& cameraLocation, // the "eye" point

   bool isDashed = false
);

// What a client means by "height" of a font
// depends on the client's needs and what they want to draw.
// This allows the client to specify what they mean.
//
// The DU prefix is for DrawUtil
enum DU_FontHeightType {
   DU_FONT_ASCENT, // from baseline to top; good for chars [A-Z0-9] (except Q)
   DU_FONT_ASCENT_PLUS_DESCENT, // includes descenders, as in chars [jgpqy]
   DU_FONT_TOTAL_HEIGHT // ascent + descent + recommended vertical spacing
};

// returns the width of a string given the desired height
float widthOfStringToBeDrawn(
   const char * buffer,// the string
   float height,       // string is scaled to be this high, in world space units
   DU_FontHeightType fontHeightType,
   bool useTextureFont
);

// To draw a string that is always facing toward the front,
// pass in ``(camera->getPosition()-origin).normalized()'' for facingDirection,
// and ``camera->getUp()'' for up.
void drawString(
   const Point3 & origin, // lower left corner of string
   const char * buffer,
   float height,          // desired height of string, in world space units
   DU_FontHeightType fontHeightType,

   // a unit vector; the string will face forward this way
   const Vector3 & facingDirection,

   const Vector3 & up, // a unit vector

   bool isBlended = true,  // drawn with alpha blended antialiasing
   float thickness = 1.0f, // set to >1 for bolder text
   bool useTextureFont = false
);


#endif /* DRAWUTIL_H */

