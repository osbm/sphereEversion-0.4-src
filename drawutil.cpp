
#include "drawutil.h"
#include "fontdata.h"
#include <GL/glut.h>
#include <string.h>   /* for strlen() */


// From the manual page for glutStrokeCharacter():
// The Mono Roman font supported by GLUT has characters that are
// 104.76 units wide, up to 119.05 units high, and descenders can
// go as low as 33.33 units.
//
// The "G_" prefix is because it is specific to GLUT.
// We don't use the prefix "GLUT_", because that's
// reserved for stuff that GLUT defines itself.
static const float G_FONT_ASCENT = 119.05f;
static const float G_FONT_DESCENT = 33.33f;
static const float G_CHAR_WIDTH = 104.76f;
// This is the recommended spacing;
// it's chosen to match that in the texture font that is 18 pixels high,
// and uses 1 row of pixels in every character bitmap for vertical spacing.
static const float G_FONT_VERTICAL_SPACE
   = (1.0f/17)*(G_FONT_ASCENT+G_FONT_DESCENT);


// If defined, dashing is done by skipping every 3rd segment.
// If not defined, dashing is done by skipping every 2nd segment.
//#define DASHED_3


void drawBox(
   const AlignedBox& box0, bool expand, bool isCool
) {
   AlignedBox box( box0 );

   // expand the box
   if ( expand ) {
      float diagonal = box.getDiagonal().length();
      diagonal /= 30;
      Vector3 v( diagonal, diagonal, diagonal );
      box = AlignedBox( box.getMin() - v, box.getMax() + v );
   }

   // now draw it.
   if ( ! isCool ) {
      glBegin( GL_LINE_STRIP );
         glVertex3fv( box.getCorner( 0 ).get() );
         glVertex3fv( box.getCorner( 1 ).get() );
         glVertex3fv( box.getCorner( 3 ).get() );
         glVertex3fv( box.getCorner( 2 ).get() );
         glVertex3fv( box.getCorner( 6 ).get() );
         glVertex3fv( box.getCorner( 7 ).get() );
         glVertex3fv( box.getCorner( 5 ).get() );
         glVertex3fv( box.getCorner( 4 ).get() );
         glVertex3fv( box.getCorner( 0 ).get() );
         glVertex3fv( box.getCorner( 2 ).get() );
      glEnd();
      glBegin( GL_LINES );
         glVertex3fv( box.getCorner( 1 ).get() );
         glVertex3fv( box.getCorner( 5 ).get() );
         glVertex3fv( box.getCorner( 3 ).get() );
         glVertex3fv( box.getCorner( 7 ).get() );
         glVertex3fv( box.getCorner( 4 ).get() );
         glVertex3fv( box.getCorner( 6 ).get() );
      glEnd();
   }
   else {
      glBegin( GL_LINES );
         for ( int dim = 0; dim < 3; ++dim ) {
            Vector3 v = ( box.getCorner(1<<dim) - box.getCorner(0) )*0.1;
            for ( int a = 0; a < 2; ++a ) {
               for ( int b = 0; b < 2; ++b ) {
                  int i = (a << ((dim+1)%3)) | (b << ((dim+2)%3));
                  glVertex3fv( box.getCorner(i).get() );
                  glVertex3fv( ( box.getCorner(i) + v ).get() );
                  i |= 1 << dim;
                  glVertex3fv( box.getCorner(i).get() );
                  glVertex3fv( ( box.getCorner(i) - v ).get() );
               }
            }
         }
      glEnd();
   }
}

void drawCircle(
   const Point3& centre,
   const Vector3& normal,
   float radius,
   float arcLengthPerPixel,
   bool isDashed
) {
   Matrix m;
   m.setToLookAt( centre, centre+normal, normal.choosePerpendicular(), true );
   glPushMatrix();
      glMultMatrixf( m.get() );

      // We draw the circle as a polyline
      // with approximately one side per pixel.
      // The "+1" is to round up.
      int nbSides = (int)( 2 * M_PI * radius / arcLengthPerPixel + 1 );

      float deltaAngle = 2 * M_PI / nbSides;
      float angle;
      if ( isDashed ) {
         glBegin( GL_LINES );
            int i = 0;
            while ( i < nbSides ) {
               angle = i * deltaAngle;
               glVertex3f( radius * cos(angle), radius * sin(angle), 0 );
               ++i;
               angle = i * deltaAngle;
               glVertex3f( radius * cos(angle), radius * sin(angle), 0 );
#ifdef DASHED_3
               i += 2;
#else
               ++i;
#endif
            }
         glEnd();
      }
      else {
         glBegin( GL_LINE_LOOP );
            for ( int i = 0; i < nbSides; ++i ) {
               angle = i * deltaAngle;
               glVertex3f( radius * cos(angle), radius * sin(angle), 0 );
            }
         glEnd();
      }
   glPopMatrix();
}

void drawShadedCircle(
   const Point3& centre,
   const Vector3& normal,
   float radius,
   float arcLengthPerPixel,
   float farThickness,
   float nearThickness,
   const Point3& farColour,
   const Point3& nearColour,
   const Point3& cameraLocation,
   bool isDashed,
   bool isArc,
   const Vector3 & radialVectorAtWhichArcStarts,
   float arcAngle
) {
   Matrix m,mInverse;
   Vector3 radialVector = isArc
      ? radialVectorAtWhichArcStarts : normal.choosePerpendicular();
   m.setToLookAt( centre, centre+normal, radialVector, true );
   mInverse.setToLookAt( centre, centre+normal, radialVector, false );
   glPushMatrix();
      glMultMatrixf( m.get() );

      // Compute the direction vector that points from the circle's
      // centre to the camera, and do so in the local frame of reference.
      // Note: the circle's centre is at Point3(0,0,0) in the local frame.
      Vector3 viewDirection
         = Vector3(mInverse*cameraLocation).normalized();

      // We draw the circle as a polyline
      // with approximately one side per pixel.
      // The "+1" is to round up.
      int nbSides = (int)( 2 * M_PI * radius / arcLengthPerPixel + 1 );

      float deltaAngle = 2 * M_PI / nbSides;
      int i_max = nbSides;
      if ( isArc ) {
         ASSERT( arcAngle >= 0 );
         i_max = ROUND( i_max * arcAngle / (2 * M_PI) );
         if ( i_max > nbSides ) i_max = nbSides;
      }
      Point3 oldPoint, newPoint( 0, radius, 0 );
      // Note: although glColor() can be called inside a glBegin()/glEnd()
      // block, glLineWidth() cannot.
      for ( int i = 1; i <= i_max; ++i ) {
         oldPoint = newPoint;
         float angle = i * deltaAngle;

         // We want ``radialVector'' (the +y axis in our local space)
         // to correspond to an angle of zero, and for the angle
         // to increase counterclockwise around the ``normal''
         // (the -z axis in our local space).
         newPoint = Point3( radius * sin(angle), radius * cos(angle), 0 );

#ifdef DASHED_3
         if ( isDashed && ((i%3) != 1) ) {
            // For dashed lines, line segments are drawn when i is 1, 4, 7, ...
#else
         if ( isDashed && ((i%2) != 1) ) {
#endif
            // For other values, we skip (continue).
            // This is a simple way of ensuring that oldPoint will be
            // correct when the time comes to draw a line segment.
            continue;
         }
         float weight = Vector3(newPoint).normalized()*viewDirection;
         // now, weight is in [-1,1]
         weight = 0.5f*(weight+1);
         // now, weight is in [0,1].  1 corresponds to "near", 0 to "far".
         Point3 colour = farColour + (nearColour-farColour)*weight;
         glColor3fv( colour.get() );
         glLineWidth( farThickness + (nearThickness-farThickness)*weight );
         glBegin( GL_LINES );
            glVertex3fv( oldPoint.get() );
            glVertex3fv( newPoint.get() );
         glEnd();
      }
   glPopMatrix();
}

void drawCone(
   const Point3& apex,
   const Vector3& axis,
   float semiAngle,
   float lateralLength,
   int numLateralSides,
   bool drawBase
) {
   Matrix m;
   m.setToLookAt( apex, apex+axis, axis.choosePerpendicular(), true );
   glPushMatrix();
      glMultMatrixf( m.get() );
      float h = lateralLength * cos( semiAngle );
      float r = lateralLength * sin( semiAngle );
      Point3 * basePoints = new Point3[ numLateralSides ];
      float deltaAngle = 2*M_PI / numLateralSides;
      int i;
      glBegin( GL_LINES );
         for ( i = 0; i < numLateralSides; ++i ) {
            float angle = i * deltaAngle;
            basePoints[i] = Point3( r*cos(angle), r*sin(angle), -h );
            glVertex3f( 0, 0, 0 );
            glVertex3fv( basePoints[i].get() );
         }
      glEnd();
      if ( drawBase ) {
         glBegin( GL_LINE_LOOP );
            for ( i = 0; i < numLateralSides; ++i ) {
               glVertex3fv( basePoints[i].get() );
            }
         glEnd();
      }
      delete [] basePoints;  basePoints = 0;
   glPopMatrix();
}

void drawFrame(
   const Point3& origin,
   const Vector3& i, const Vector3& j, const Vector3& k,
   float length, bool drawArrowHeads, bool isColoured,
   const Point3& i_colour,
   const Point3& j_colour,
   const Point3& k_colour
) {
   Point3 A = origin + (i*length);
   Point3 B = origin + (j*length);
   Point3 C = origin + (k*length);
   glBegin( GL_LINES );
      if ( isColoured ) glColor3fv( i_colour.get() );
      glVertex3fv( origin.get() );
      glVertex3fv( A.get() );
      if ( isColoured ) glColor3fv( j_colour.get() );
      glVertex3fv( origin.get() );
      glVertex3fv( B.get() );
      if ( isColoured ) glColor3fv( k_colour.get() );
      glVertex3fv( origin.get() );
      glVertex3fv( C.get() );
   glEnd();
   if ( drawArrowHeads ) {
      drawCone( C, -k, M_PI/8, length/7, 3, false );
      if ( isColoured ) glColor3fv( j_colour.get() );
      drawCone( B, -j, M_PI/8, length/7, 3, false );
      if ( isColoured ) glColor3fv( i_colour.get() );
      drawCone( A, -i, M_PI/8, length/7, 3, false );
   }
}

void drawCrossHairs( const Point3& centre, float radius ) {
   const float& r = radius;
   glBegin( GL_LINES );
      glVertex3fv( ( centre + Vector3(-r, 0, 0 ) ).get() );
      glVertex3fv( ( centre + Vector3( r, 0, 0 ) ).get() );
      glVertex3fv( ( centre + Vector3( 0,-r, 0 ) ).get() );
      glVertex3fv( ( centre + Vector3( 0, r, 0 ) ).get() );
      glVertex3fv( ( centre + Vector3( 0, 0,-r ) ).get() );
      glVertex3fv( ( centre + Vector3( 0, 0, r ) ).get() );
   glEnd();
}

void drawRay( const Ray& ray, float length, float arrowHeadLength ) {
   const Point3& p = ray.origin;
   const Vector3& v = ray.direction;
   glBegin( GL_LINES );
      glVertex3fv( p.get() );
      glVertex3fv( ( p + v*length ).get() );
   glEnd();
   drawCone( p+v*length, -v, M_PI/8, v.length()*arrowHeadLength, 5, false );
}

void drawShadedRay(
   const Ray& ray, float length, float arrowHeadLength,
   float farThickness,
   float nearThickness,
   const Point3& farColour,
   const Point3& nearColour,
   const Point3& cameraLocation,
   bool isDashed
) {
   static const int NUM_SEGMENTS = 20; // FIXME: maybe this should depend on the apparent size (i.e. length in pixels) of the ray
   const Point3& origin = ray.origin;
   const Vector3& direction = ray.direction;
   float dot = direction.normalized()
      * ( cameraLocation - origin ).normalized();
   Point3 oldPoint, newPoint( origin );
   // Note: although glColor() can be called inside a glBegin()/glEnd()
   // block, glLineWidth() cannot.
   for ( int i = 1; i <= NUM_SEGMENTS; ++i ) {
      oldPoint = newPoint;
      float u = i/(float)NUM_SEGMENTS;
      newPoint = origin + direction*u*length;
#ifdef DASHED_3
      if ( isDashed && ((i%3) != 1) ) {
         // For dashed lines, line segments are drawn when i is 1, 4, 7, ...
#else
      if ( isDashed && ((i%2) != 1) ) {
#endif
         // For other values, we skip (continue).
         // This is a simple way of ensuring that oldPoint will be
         // correct when the time comes to draw a line segment.
         continue;
      }

      // map u to a parameter v.
      // If the dot product is 1, we want v = u.
      // If the dot product is 0, we want v = 1/2.
      // If the dot product is -1, we want v = -u.
      // For intermediate values of the dot product,
      // we want linear interpolation.
      float v = u*dot + 0.5f*(1-dot);
      // Both u and v should be in [0,1].

      Point3 colour = farColour + (nearColour-farColour)*v;
      glColor3fv( colour.get() );
      glLineWidth( farThickness + (nearThickness-farThickness)*v );

      glBegin( GL_LINES );
         glVertex3fv( oldPoint.get() );
         glVertex3fv( newPoint.get() );
      glEnd();
   }
   drawCone( origin+direction*length, -direction,
      M_PI/8, direction.length()*arrowHeadLength, 5, false
   );
}

float widthOfStringToBeDrawn(
   const char * buffer,
   float height,
   DU_FontHeightType fontHeightType,
   bool useTextureFont
) {
   if ( buffer == 0 ) return 0;

   float h, w_over_h;
   if ( useTextureFont ) {
      switch ( fontHeightType ) {
         case DU_FONT_ASCENT :
            h = FontData::getCharAscent();
            break;
         case DU_FONT_ASCENT_PLUS_DESCENT :
         case DU_FONT_TOTAL_HEIGHT :
         default:
            h = FontData::getCharAscent() + FontData::getCharDescent();
            break;
      }
      w_over_h = FontData::getCharWidth() / h;
   }
   else {
      switch ( fontHeightType ) {
         case DU_FONT_ASCENT :
            h = G_FONT_ASCENT;
            break;
         case DU_FONT_ASCENT_PLUS_DESCENT :
            h = G_FONT_ASCENT + G_FONT_DESCENT;
            break;
         case DU_FONT_TOTAL_HEIGHT :
         default:
            h = G_FONT_ASCENT + G_FONT_DESCENT + G_FONT_VERTICAL_SPACE;
            break;
      }
      w_over_h = G_CHAR_WIDTH / h;
   }

   return height * strlen( buffer ) * w_over_h;
}

void drawString(
   const Point3 & origin,
   const char * buffer,
   float height,
   DU_FontHeightType fontHeightType,
   const Vector3 & facingDirection,
   const Vector3 & up,
   bool isBlended,
   float thickness,
   bool useTextureFont
) {
   if ( buffer == 0 ) return;

   Vector3 forwardDirection = -facingDirection;
   //Vector3 right = forwardDirection ^ up;
   Matrix m;
   m.setToLookAt( origin, origin + forwardDirection, up, true );
   glPushMatrix();
      glMultMatrixf( m.get() );

      if ( useTextureFont ) {
         // FIXME when ``height'' is smaller than the font's height,
         // some texels don't get rendered, and the text can be illegible.
         // Maybe in such cases, it would be better to render each
         // opaque texel as a GL_POINT, or even a small quad,
         // rather than rendering a textured quad for each character.

         glBindTexture( GL_TEXTURE_2D, FontData::getTextureName() );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
            isBlended ? GL_LINEAR : GL_NEAREST );
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
            isBlended ? GL_LINEAR : GL_NEAREST );
         glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
            // Each fragment's alpha will be multiplied
            // by the alpha value in the texture map.
            GL_MODULATE
         );
         glEnable( GL_TEXTURE_2D );
         glEnable( GL_BLEND );
         glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
         glShadeModel( GL_FLAT );

         if ( fontHeightType == DU_FONT_ASCENT ) {
            float true_height = height
               * (FontData::getCharAscent()+FontData::getCharDescent())
               / (float)FontData::getCharAscent();

            // move origin down from baseline to bottom
            glTranslatef( 0, height-true_height, 0 );

            height = true_height;
         }
         float width = height * FontData::getCharWidth()
            / (float)(FontData::getCharAscent()+FontData::getCharDescent());

         float tc_x1, tc_y1; //texture coordinates of upper left corner of char
         float tc_x2, tc_y2; //texture coordinates of lower right corner of char
         glBegin( GL_QUADS );
            for ( int j = 0; buffer[j] != '\0'; ++j ) {
               FontData::getTextureCoords(
                  buffer[j], tc_x1, tc_y1, tc_x2, tc_y2
               );

               glTexCoord2f(tc_x1,tc_y2); // lower left corner
               glVertex2f(j*width,0);
               glTexCoord2f(tc_x2,tc_y2); // lower right corner
               glVertex2f(j*width+width,0);
               glTexCoord2f(tc_x2,tc_y1); // upper right corner
               glVertex2f(j*width+width,height);
               glTexCoord2f(tc_x1,tc_y1); // upper left corner
               glVertex2f(j*width,height);
            }
         glEnd();
         glDisable( GL_BLEND );
         glDisable( GL_TEXTURE_2D );
      }
      else {
         float ascent; // in world space units
         switch ( fontHeightType ) {
            case DU_FONT_ASCENT :
               ascent = height;
               break;
            case DU_FONT_ASCENT_PLUS_DESCENT :
               ascent = height * G_FONT_ASCENT
                  / ( G_FONT_ASCENT + G_FONT_DESCENT );
               break;
            case DU_FONT_TOTAL_HEIGHT :
            default:
               ascent = height * G_FONT_ASCENT
                  / ( G_FONT_ASCENT + G_FONT_DESCENT + G_FONT_VERTICAL_SPACE );
               break;
         }
         // shift the origin to the baseline of the font
         glTranslatef( 0, height - ascent, 0 );

         if ( isBlended ) {
            // This will draw the text with anti-aliased strokes,
            // making it easier to read small text.
            glEnable( GL_LINE_SMOOTH );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glEnable( GL_BLEND );
         }
         if ( thickness != 1 )
            glLineWidth( thickness );

         float s = ascent / G_FONT_ASCENT; // scale factor
         glScalef( s, s, 1 );
         for ( int j = 0; buffer[j] != '\0'; ++j )
            glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, buffer[j] );

         if ( thickness != 1 )
            glLineWidth( 1 );
         if ( isBlended ) {
            glDisable( GL_LINE_SMOOTH );
            glDisable( GL_BLEND );
         }
      }
   glPopMatrix();
}

