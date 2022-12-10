
#ifndef FONTDATA_H
#define FONTDATA_H


class FontData {
private:
   static int _charWidth; // in pixels
   static int _charHeight; // in pixels, is equal to ascent + descent
   static int _charAscent; // in pixels
   static int _charDescent; // in pixels; includes vertical spacing
   static int _numChars;
   static unsigned char * _bitmap;  // one bitmap of the entire character set
   static int _bitmapWidth;
   static int _bitmapHeight;
   static int _numColumnsOfCharsInBitmap;
   static bool _isBitmapInitialized;

   static unsigned int _textureName;
   static bool _isTextureInitialized;
   static void initializeTexture();
public:
   // If the client doesn't call this,
   // it will be called lazily,
   // the first time any other static methods are called
   // that require access to the bitmap.
   static void initialize();

   // The client should call this.
   static void finalize() {
      delete [] _bitmap;
      _bitmap = 0;
      _isTextureInitialized = false;
   }

   static int getCharWidth() { return _charWidth; }
   static int getCharHeight() { return _charHeight; }
   static int getCharAscent() { return _charAscent; }
   static int getCharDescent() { return _charDescent; }

   // Returns a handle to the bitmap
   // loaded in texture memory.
   // The first time this is called,
   // it must be called within an OpenGL context.
   static unsigned int getTextureName() {
      if ( ! _isTextureInitialized )
         initializeTexture();
      return _textureName;
   }

   // Passes back corners of region within texture
   // that contain the given character.
   static void getTextureCoords(
      char c, float & x1, float & y1, float & x2, float & y2
   ) {
      int charIndex;
      if ( c < ' ' || c > '~' ) charIndex = _numChars-1; // "error" character
      else charIndex = c - ' ';
      int x0 = (charIndex % _numColumnsOfCharsInBitmap)*(_charWidth+2);
      int y0 = (charIndex / _numColumnsOfCharsInBitmap)*(_charHeight+2);
      x1 = (x0+1)/(float)_bitmapWidth;
      x2 = (x0+1+_charWidth)/(float)_bitmapWidth;
      y1 = (y0+1)/(float)_bitmapHeight;
      y2 = (y0+1+_charHeight)/(float)_bitmapHeight;
   }
};


#endif /* FONTDATA_H */

