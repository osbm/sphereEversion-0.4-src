
#include "fontdata.h"
#include "fontDefinition.h"
#include "global.h"
#ifdef _WIN32
#include <GL/glut.h>
#else
#include <GL/gl.h>
#endif
#include <math.h>


int FontData::_charWidth = FONT_CHAR_WIDTH;
int FontData::_charHeight = FONT_CHAR_HEIGHT;
int FontData::_charAscent = FONT_CHAR_ASCENT;
int FontData::_charDescent = FONT_CHAR_DESCENT;
int FontData::_numChars = FONT_NUM_CHARS;
unsigned char * FontData::_bitmap = 0;
int FontData::_bitmapWidth = 0;
int FontData::_bitmapHeight = 0;
int FontData::_numColumnsOfCharsInBitmap = 0;
bool FontData::_isBitmapInitialized = false;
unsigned int FontData::_textureName = 0;
bool FontData::_isTextureInitialized = false;


void FontData::initializeTexture() {
   if ( ! _isBitmapInitialized )
      initialize();
   ASSERT( _isBitmapInitialized );
   if ( _isTextureInitialized )
      return;
   _isTextureInitialized = true;

   glGenTextures( 1, &_textureName );
   glBindTexture( GL_TEXTURE_2D, _textureName );
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   glTexImage2D(
      GL_TEXTURE_2D,     // target
      0,                 // level
      GL_ALPHA,          // internal format (i.e. components)
      _bitmapWidth,      // width (must be a power of 2)
      _bitmapHeight,     // height (must be a power of 2)
      0,                 // border width
      GL_ALPHA,          // format
      GL_UNSIGNED_BYTE,  // type
      _bitmap            // pixels
   );

   // Calling these aren't neccessary,
   // and may need to be called again by the client anyway,
   // if the client uses other 2D textures,
   // or uses the font texture in an unintended way.
   // But we call these to at least setup a nice default state.
   // (The below code also provides an example
   // to client programmers who have access to this source code.)
   glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
      // Each fragment's alpha will be multiplied
      // by the alpha value in the texture map.
      GL_MODULATE
   );
}

void FontData::initialize() {
   if ( _isBitmapInitialized )
      return;
   _isBitmapInitialized = true;

   // Construct a bitmap to store all characters.

   // Determine the dimensions of the bitmap,
   // which must be powers of 2,
   // so it can be loaded as a texture.
   //
   // Characters are arranged in multiple rows,
   // in such a way as to make the bitmap approximately squarish.
   // Although it would be simpler to arrange the
   // characters in a single row, making the bitmap
   // wide and short, this would lead to two potential problems:
   //  - 1st, if the bitmap is loaded as a texture,
   //    the texture coordinates must lie between 0 and 1.
   //    A single row of characters would mean that
   //    we need pretty good precision in the x texture coordinates
   //    (possibly more than OpenGL supports)
   //    to resolve individual texels,
   //    and at the same time the precision in the y coordinate
   //    would be underutilized.
   //  - 2nd, we must round up the dimensions of the bitmap to powers of 2.
   //    If packing the characters into a single row
   //    requires, say, 1025 pixels horizontally,
   //    this leads to a *very* wide bitmap of 2048 pixels,
   //    which would exacerbate the first problem.
   // Let N be the total number of characters,
   // that are each W pixels wide and H pixels tall,
   // and let C and R==N/C be the number of columns and rows, respectively.
   // To make the bitmap approximately squarish,
   // we want
   //    C*W == R*H
   //    C*W == (N/C)*H
   //    C == sqrt( N*H/W )
   // We would like the bitmap to have each character be surrounded
   // by a 1-pixel margin of blank texels.  Without this, sometimes
   // non-blank texels at the edge of one character can bleed
   // onto the edge of another character being rendered.
   // So, we adjust by adding margins to the character dimensions:
   //    C*(W+2) == R*(H+2)
   //    C*(W+2) == (N/C)*(H+2)
   //    C == sqrt( N*(H+2)/(W+2) )
   _numColumnsOfCharsInBitmap = (int)ceil( sqrt(
      _numChars * (_charHeight+2) / (float)(_charWidth+2)
   ) );
   int numRowsOfCharsInBitmap = (int)ceil(
      _numChars / _numColumnsOfCharsInBitmap
   );
   _bitmapWidth = 2;
   _bitmapHeight = 2;
   while ( _bitmapWidth < _numColumnsOfCharsInBitmap * (_charWidth+2) )
      _bitmapWidth <<= 1;
   while ( _bitmapHeight < numRowsOfCharsInBitmap * (_charHeight+2) )
      _bitmapHeight <<= 1;

   // Allocate and clear the bitmap.
   _bitmap = new unsigned char[ _bitmapWidth * _bitmapHeight ];
   int x,y;
   for ( x = 0; x < _bitmapWidth; ++x )
      for ( y = 0; y < _bitmapHeight; ++y )
         _bitmap[x+y*_bitmapWidth] = 0;

   // Write the characters into the bitmap.
   int charIndex;
   for ( charIndex = 0; charIndex < _numChars; ++charIndex ) {
      int x_offset = (charIndex % _numColumnsOfCharsInBitmap)*(_charWidth+2);
      int y_offset = (charIndex / _numColumnsOfCharsInBitmap)*(_charHeight+2);
      for ( x = 0; x < _charWidth; ++x )
         for ( y = 0; y < _charHeight; ++y )
            if ( FONT_DEFINITION[charIndex][y][x] != '.' )
               _bitmap[ (x+1+x_offset) + (y+1+y_offset)*_bitmapWidth ] = 255;
   }
}

