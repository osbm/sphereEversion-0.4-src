
#include "drawutil2D.h"
#include "fontdata.h"
#ifdef _WIN32
#include "global.h"  /* for M_PI */
#endif
#include <GL/glut.h>
#include <math.h>
#include <string.h>


/* #define USE_HACKISH_OFFSET */


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


bool OpenGL2DInterface::_useTextureFont = false;


void OpenGL2DInterface::resize( int w, int h ) {
   _window_width = w;
   _window_height = h;
   // Check out appendix H of the OpenGL Programming Guide for the
   // rationale behind this code.
   glViewport( 0, 0, w, h );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glOrtho( 0, w, 0, h, -1, 1 );
#ifdef USE_HACKISH_OFFSET
   glTranslatef( 0.375f, 0.375f, 0 );
#endif
}

void OpenGL2DInterface::pushProjection( int w, int h ) {
   _window_width = w;
   _window_height = h;
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   glOrtho( 0, w, 0, h, -1, 1 );
#ifdef USE_HACKISH_OFFSET
   glTranslatef( 0.375f, 0.375f, 0 );
#endif
   glMatrixMode( GL_MODELVIEW );
}

void OpenGL2DInterface::popProjection() const {
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();
   glMatrixMode( GL_MODELVIEW );
}

float OpenGL2DInterface::convertPixelX( int x ) const {
#ifdef USE_HACKISH_OFFSET
   return x;
#else
   return x+0.5f;
#endif
}

float OpenGL2DInterface::convertPixelY( int y ) const {
   y = _window_height - y - 1;
#ifdef USE_HACKISH_OFFSET
   return y;
#else
   return y+0.5f;
#endif
}

void OpenGL2DInterface::plotPixel( int x, int y ) const {
   y = _window_height - y - 1;
   glBegin( GL_POINTS );
#ifdef USE_HACKISH_OFFSET
      glVertex2i( x, y );
#else
      glVertex2f( x+0.5f, y+0.5f );
#endif
   glEnd();
}

void OpenGL2DInterface::drawLine( int x1, int y1, int x2, int y2 ) const {
   y1 = _window_height - y1 - 1;
   y2 = _window_height - y2 - 1;
   glBegin( GL_LINES );
#ifdef USE_HACKISH_OFFSET
      glVertex2i( x1, y1 );
      glVertex2i( x2, y2 );
#else
      glVertex2f( x1+0.5f, y1+0.5f );
      glVertex2f( x2+0.5f, y2+0.5f );
#endif
   glEnd();
   glBegin( GL_POINTS );
#ifdef USE_HACKISH_OFFSET
      glVertex2i( x2, y2 );
#else
      glVertex2f( x2+0.5f, y2+0.5f );
#endif
   glEnd();
}

void OpenGL2DInterface::drawRect( int x, int y, int w, int h ) const {
   y = _window_height - y - h;
   --w;
   --h;
   glBegin( GL_LINE_LOOP );
#ifdef USE_HACKISH_OFFSET
      glVertex2i( x, y );
      glVertex2i( x+w, y );
      glVertex2i( x+w, y+h );
      glVertex2i( x, y+h );
#else
      glVertex2f( x+0.5f, y+0.5f );
      glVertex2f( x+w+0.5f, y+0.5f );
      glVertex2f( x+w+0.5f, y+h+0.5f );
      glVertex2f( x+0.5f, y+h+0.5f );
#endif
   glEnd();
}

void OpenGL2DInterface::fillRect( int x, int y, int w, int h ) const {
   y = _window_height - y - h;
#if 0
   glBegin( GL_QUADS );
      glVertex2i( x, y );
      glVertex2i( x+w, y );
      glVertex2i( x+w, y+h );
      glVertex2i( x, y+h );
   glEnd();
#else
   glRecti( x, y, x+w, y+h );
#endif
}

void OpenGL2DInterface::drawCircle(
   int x, int y, int radius, bool filled
) const {
   y = _window_height - y - 1;
   if ( filled ) {
      glBegin( GL_TRIANGLE_FAN );
      glVertex2f( x+0.5f, y+0.5f );
   }
   else glBegin( GL_LINE_LOOP );
      int numSides = (int)( 2 * M_PI * radius + 1 );
      float deltaAngle = 2 * M_PI / numSides;

      // Note: I used to loop up to "< numSides",
      // and indeed I think this is okay for the
      // non-filled case (because of the GL_LINE_LOOP),
      // however when used for drawing a triangle fan,
      // rather than produce a complete disc,
      // we have a 1-pixel-wide sliver missing.
      // Using "<=" fixes this.
      for ( int i = 0; i <= numSides; ++i ) {
         float angle = i * deltaAngle;
         glVertex2f( x+radius*cos(angle)+0.5f, y+radius*sin(angle)+0.5f );
      }
   glEnd();
}

void OpenGL2DInterface::fillCircle( int x, int y, int radius ) const {
   drawCircle( x, y, radius, true );
}

void OpenGL2DInterface::drawArc(
   int x, int y, int radius,
   float startAngle,
   float arcAngle,
   bool filled,
   bool isArrowHeadAtStart,
   bool isArrowHeadAtEnd,
   float arrowHeadLength
) {
   y = _window_height - y - 1;
   if ( filled ) {
      glBegin( GL_TRIANGLE_FAN );
      glVertex2f( x+0.5f, y+0.5f );
   }
   else glBegin( GL_LINE_STRIP );
      int numSides = (int)( fabs(arcAngle) * radius + 1 );
      float deltaAngle = arcAngle / numSides;

      for ( int i = 0; i <= numSides; ++i ) {
         float angle = startAngle + i * deltaAngle;
         glVertex2f( x+radius*cos(angle)+0.5f, y+radius*sin(angle)+0.5f );
      }
   glEnd();

   if ( isArrowHeadAtStart || isArrowHeadAtEnd ) {

      // An arrow head (pointing down) before rotation:
      //
      //      A     B
      //       \   /
      //        \ /
      //   ------P---- x axis
      //
      // The 3 points forming the arrow head are A, P, B.
      // To make the arrow head point up, invert the signs
      // of A and B's y coordinates.
      //
      float Px = radius;
      float Ax = radius - arrowHeadLength/M_SQRT2;
      float Bx = radius + arrowHeadLength/M_SQRT2;
      float Py = 0;
      float Ay = arrowHeadLength/M_SQRT2;
      float By = arrowHeadLength/M_SQRT2;

      // Draw arrow head at one end.

      if ( arcAngle < 0 ) {
         // invert the direction of the arrow
         Ay = -Ay;
         By = -By;
      }
      if ( isArrowHeadAtStart ) {
         float theta = - startAngle;
         float c = cos( theta );
         float s = sin( theta );
         float Px_prime =  c*Px + s*Py;
         float Py_prime = -s*Px + c*Py;
         float Ax_prime =  c*Ax + s*Ay;
         float Ay_prime = -s*Ax + c*Ay;
         float Bx_prime =  c*Bx + s*By;
         float By_prime = -s*Bx + c*By;
         glBegin( GL_LINE_STRIP );
            glVertex2f( x+Ax_prime+0.5f, y+Ay_prime+0.5f );
            glVertex2f( x+Px_prime+0.5f, y+Py_prime+0.5f );
            glVertex2f( x+Bx_prime+0.5f, y+By_prime+0.5f );
         glEnd();
      }

      // Draw arrow head at other end.

      // invert the direction of the arrow
      Ay = -Ay;
      By = -By;

      if ( isArrowHeadAtEnd ) {
         float theta = - ( startAngle + arcAngle );
         float c = cos( theta );
         float s = sin( theta );
         float Px_prime =  c*Px + s*Py;
         float Py_prime = -s*Px + c*Py;
         float Ax_prime =  c*Ax + s*Ay;
         float Ay_prime = -s*Ax + c*Ay;
         float Bx_prime =  c*Bx + s*By;
         float By_prime = -s*Bx + c*By;
         glBegin( GL_LINE_STRIP );
            glVertex2f( x+Ax_prime+0.5f, y+Ay_prime+0.5f );
            glVertex2f( x+Px_prime+0.5f, y+Py_prime+0.5f );
            glVertex2f( x+Bx_prime+0.5f, y+By_prime+0.5f );
         glEnd();
      }
   }
}

void OpenGL2DInterface::drawShadedArc(
   int x, int y, int radius,
   float startAngle,
   float arcAngle,
   float Ri, float Gi, float Bi, float Ai,
   float Rf, float Gf, float Bf, float Af,
   bool filled
) {
   y = _window_height - y - 1;

   if ( filled ) glBegin( GL_TRIANGLES );
   else glBegin( GL_LINES );

      int numSides = (int)( fabs(arcAngle) * radius + 1 );
      float deltaAngle = arcAngle / numSides;

      for ( int i = 0; i < numSides; ++i ) {
         float uf = i/(float)(numSides-1); // varies from 0 to 1
         float ui = 1-uf;
         glColor4f( ui*Ri+uf*Rf, ui*Gi+uf*Gf, ui*Bi+uf*Bf, ui*Ai+uf*Af );

         float angle = startAngle + i * deltaAngle;
         if ( filled ) glVertex2f( x+0.5f, y+0.5f );
         glVertex2f( x+radius*cos(angle)+0.5f, y+radius*sin(angle)+0.5f );
         glVertex2f(
            x+radius*cos(angle+deltaAngle)+0.5f,
            y+radius*sin(angle+deltaAngle)+0.5f
         );
      }

   glEnd();
}

void OpenGL2DInterface::drawImage(
   int x1, int y1,
   const unsigned char * image,
   int width, int height, int numComponents
) const {

#if 1 /* use glDrawPixels() -- fast, but entire image is culled if raster pos is outside viewport.  A more flexible (but somewhat less efficient) alternative would be to render a textured quad. */

#if 0
   // Draws image upside down.  Furthermore, entire image is culled
   // if *lower* left corner is outside viewport.
   glRasterPos2i( x1, _window_height - height - y1 );
#else
   // Draws image right-side up.  However, entire image is culled
   // if *upper* left corner is outside viewport.
   glPixelZoom( 1.0f, -1.0f );
   glRasterPos2i( x1, _window_height - y1 );
#endif

   glPixelStorei( GL_PACK_ALIGNMENT, 1 );
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   if ( numComponents == 1 )
      glDrawPixels( width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
   else if ( numComponents == 3 )
      glDrawPixels( width, height, GL_RGB, GL_UNSIGNED_BYTE, image );
   else if ( numComponents == 4 )
      glDrawPixels( width, height, GL_RGBA, GL_UNSIGNED_BYTE, image );

#else /* plot individual GL_POINTS -- slow, but should always work */

   glBegin( GL_POINTS );
      if ( numComponents == 1 ) {
         unsigned char c;
         for ( int y = 0; y < height; ++y ) {
            for ( int x = 0; x < width; ++x ) {
               c = image[y*width+x];
               glColor3ub( c, c, c );
               glVertex2f( x1+x+0.5f, (_window_height - y-y1 - 1)+0.5f );
            }
         }
      }
      else if ( numComponents == 3 ) {
         for ( int y = 0; y < height; ++y ) {
            for ( int x = 0; x < width; ++x ) {
               glColor3ubv( &image[(y*width+x)*3 ] );
               glVertex2f( x1+x+0.5f, (_window_height - y-y1 - 1)+0.5f );
            }
         }
      }
   glEnd();

#endif
}

void OpenGL2DInterface::drawImage(
   int x1, int y1,
   const float * image,
   int width, int height, int numComponents,
   float min, float max
) const {
   // FIXME: this should be re-implemented using glDrawPixels().
   // To support the mapping from [min,max] to [0,1], use
   // glPixelTransfer()

   float delta = max-min;
   glBegin( GL_POINTS );
      if ( numComponents == 1 ) {
         for ( int y = 0; y < height; ++y ) {
            for ( int x = 0; x < width; ++x ) {
               float f = (image[y*width+x]-min)/delta;
               glColor3f( f, f, f );
               glVertex2f( x1+x+0.5f, (_window_height - y-y1 - 1)+0.5f );
            }
         }
      }
      else if ( numComponents == 3 ) {
         for ( int y = 0; y < height; ++y ) {
            for ( int x = 0; x < width; ++x ) {
               float f1 = (image[(y*width+x)*3  ]-min)/delta;
               float f2 = (image[(y*width+x)*3+1]-min)/delta;
               float f3 = (image[(y*width+x)*3+2]-min)/delta;
               glColor3f( f1, f2, f3 );
               glVertex2f( x1+x+0.5f, (_window_height - y-y1 - 1)+0.5f );
            }
         }
      }
   glEnd();
}

float OpenGL2DInterface::stringWidthInPixels(
   const char * buffer, float height, FontHeightType fontHeightType
) {
   if ( buffer == 0 ) return 0;

   float h, w_over_h;
   if ( _useTextureFont ) {
      switch ( fontHeightType ) {
         case FONT_ASCENT :
            h = FontData::getCharAscent();
            break;
         case FONT_ASCENT_PLUS_DESCENT :
         case FONT_TOTAL_HEIGHT :
         default:
            h = FontData::getCharAscent() + FontData::getCharDescent();
            break;
      }
      w_over_h = FontData::getCharWidth() / h;
   }
   else {
      switch ( fontHeightType ) {
         case FONT_ASCENT :
            h = G_FONT_ASCENT;
            break;
         case FONT_ASCENT_PLUS_DESCENT :
            h = G_FONT_ASCENT + G_FONT_DESCENT;
            break;
         case FONT_TOTAL_HEIGHT :
         default:
            h = G_FONT_ASCENT + G_FONT_DESCENT + G_FONT_VERTICAL_SPACE;
            break;
      }
      w_over_h = G_CHAR_WIDTH / h;
   }

   return height * strlen( buffer ) * w_over_h;
}

void OpenGL2DInterface::drawString(
   int x, int y,
   const char * buffer,
   float height,
   bool isBlended,
   float thickness,
   FontHeightType fontHeightType
) const {
   if ( buffer == 0 ) return;

   y = _window_height - y - 1;

   if ( _useTextureFont ) {
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

      float y_ = y; // just to convert y to floating-point type
      if ( fontHeightType == FONT_ASCENT ) {
         float true_height = height
            * (FontData::getCharAscent()+FontData::getCharDescent())
            / (float)FontData::getCharAscent();
         y_ = y_+height-true_height; // move y down from baseline to bottom
         height = true_height;
      }
      float width = height * FontData::getCharWidth()
         / (float)(FontData::getCharAscent()+FontData::getCharDescent());

      float tc_x1, tc_y1; // texture coordinates of upper left corner of char
      float tc_x2, tc_y2; // texture coordinates of lower right corner of char
      glBegin( GL_QUADS );
         for ( int j = 0; buffer[j] != '\0'; ++j ) {
            FontData::getTextureCoords(
               buffer[j], tc_x1, tc_y1, tc_x2, tc_y2
            );

            // The (x,y) coordinates are not offset by 0.5.
            // The lower left corner of the quad falls on a pixel corner
            // (and, if the pixel dimensions of the quad are integers,
            // all corners of the quad will fall on pixel corners).
            //
            // If the client uses the font's "natural" height,
            // with blending turned off, this should cause each
            // texel to map to one pixel, exactly reproducing
            // the bitmap of the font characters.

            glTexCoord2f(tc_x1,tc_y2); // lower left corner
            glVertex2f(x+j*width,y_);
            glTexCoord2f(tc_x2,tc_y2); // lower right corner
            glVertex2f(x+j*width+width,y_);
            glTexCoord2f(tc_x2,tc_y1); // upper right corner
            glVertex2f(x+j*width+width,y_+height);
            glTexCoord2f(tc_x1,tc_y1); // upper left corner
            glVertex2f(x+j*width,y_+height);
         }
      glEnd();
      glDisable( GL_BLEND );
      glDisable( GL_TEXTURE_2D );
   }
   else {
      float ascent; // in pixels
      switch ( fontHeightType ) {
         case FONT_ASCENT :
            ascent = height;
            break;
         case FONT_ASCENT_PLUS_DESCENT :
            ascent = height * G_FONT_ASCENT
               / ( G_FONT_ASCENT + G_FONT_DESCENT );
            break;
         case FONT_TOTAL_HEIGHT :
         default:
            ascent = height * G_FONT_ASCENT
               / ( G_FONT_ASCENT + G_FONT_DESCENT + G_FONT_VERTICAL_SPACE );
            break;
      }
      float y_ = y; // just to convert y to floating-point type
      y_ = y_ + height - ascent;

      glPushMatrix();
         glTranslatef( x+0.5f, y_+0.5f, 0 );

         if ( isBlended ) {
            // This will draw the text with anti-aliased strokes,
            // making it easier to read small text.
            glEnable( GL_LINE_SMOOTH );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glEnable( GL_BLEND );
         }
         if ( thickness != 1 )
            glLineWidth( thickness );

         // We scale the text to make its height that desired by the caller.
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
      glPopMatrix();
   }
}

void OpenGL2DInterface::setScissorRect( int x, int y, int w, int h ) const {
   y = _window_height - y - h;
   glScissor( x, y, w, h );
}

