
#ifndef DRAWUTIL2D_H
#define DRAWUTIL2D_H


class OpenGL2DInterface {
private:
   // window dimensions (measured between pixel *edges*)
   int _window_width, _window_height;

   static bool _useTextureFont;
public:
   // What a client means by "height" of a font
   // depends on the client's needs and what they want to draw.
   // This allows the client to specify what they mean.
   enum FontHeightType {
      FONT_ASCENT, // from baseline to top; good for chars [A-Z0-9] (except Q)
      FONT_ASCENT_PLUS_DESCENT, // includes descenders, as in chars [jgpqy]
      FONT_TOTAL_HEIGHT // ascent + descent + recommended vertical spacing
   };

   OpenGL2DInterface() : _window_width(10), _window_height(10) {}

   static bool getUseTextureFont() { return _useTextureFont; }
   static void setUseTextureFont( bool flag ) { _useTextureFont = flag; }

   // For all of these methods, any pixel coordinates passed
   // in should be of pixel centres.
   // The upper left most pixel has its centre at the origin (0,0),
   // and x increases to the right, y increases down.
   // All dimensions should be measured between pixel edges.

   // Clients that want to use this class as the exclusive means
   // of drawing stuff can call this method (i) once to initialize stuff,
   // and (ii) every time the window is resized; to ensure the projection
   // matrix is appropriately updated.
   void resize( int w, int h );

   int getWidth() const { return _window_width; }
   int getHeight() const { return _window_height; }

   // Clients that want to, for example, use this class to draw
   // 2D graphics on top of a 3D scene, can use these methods
   // to prepare for and clean up after, respectively, drawing
   // 2D stuff.
   // The pop method ensures that the projection matrix is
   // as it was before the push.
   void pushProjection( int w, int h );
   void popProjection() const;

   // These can be used if glVertex*() must be called directly,
   // e.g.
   //    OpenGL2DInterface g;
   //    ...
   //    glVertex2f( g.convertPixelX( pixel_x ), g.convertPixelY( pixel_y ) );
   //
   float convertPixelX( int x ) const;
   float convertPixelY( int y ) const;

   void plotPixel( int x, int y ) const;
   void drawLine( int x1, int y1, int x2, int y2 ) const;
   void drawRect( int x, int y, int w, int h ) const;
   void fillRect( int x, int y, int w, int h ) const;

   void drawRectBetweenTwoCorners( int x1, int y1, int x2, int y2 ) const {
      int tmp;
      if ( x2 < x1 ) { /* swap */ tmp = x1; x1 = x2; x2 = tmp; }
      if ( y2 < y1 ) { /* swap */ tmp = y1; y1 = y2; y2 = tmp; }
      drawRect( x1, y1, x2-x1+1, y2-y1+1 );
   }

   void drawCircle(
      int x, int y, int radius, bool filled = false
   ) const;
   void fillCircle( int x, int y, int radius ) const;

   // Draws an arc (or a pie slice, if ``filled'' is true).
   void drawArc(
      int x, int y, int radius,
      float startAngle, // Relative to the +x axis, increasing anticlockwise
      float arcAngle, // In radians; can be negative
      bool filled = false,
      bool isArrowHeadAtStart = false,
      bool isArrowHeadAtEnd = false,
      float arrowHeadLength = 0
   );

   // Draws an arc (or a pie slice, if ``filled'' is true)
   // whose colour and alpha value is interpolated
   // between the given starting and ending values.
   void drawShadedArc(
      int x, int y, int radius,
      float startAngle, // Relative to the +x axis, increasing anticlockwise
      float arcAngle, // In radians; can be negative

      // The *initial*, or starting, RGBA value for the arc
      float Ri, float Gi, float Bi, float Ai,

      // The *final*, or ending, RGBA value for the arc
      float Rf, float Gf, float Bf, float Af,

      bool filled = false
   );

   void drawImage(
      int x1, int y1, // upper left corner of image
      const unsigned char * image,
      int width, int height, int numComponents
   ) const;
   void drawImage(
      int x1, int y1, // upper left corner of image
      const float * image,
      int width, int height, int numComponents,
      float min, float max // this interval is mapped to [0,1]
   ) const;

   // returns the width of a string given the desired height
   static float stringWidthInPixels(
      const char * buffer,   // the string
      float height,          // string is scaled to be this high, in pixels
      FontHeightType fontHeightType = FONT_ASCENT
   );
   void drawString(
      int x, int y,           // lower left corner of the string
      const char * buffer,    // the string
      float height,           // string is scaled to be this high, in pixels
      bool isBlended = true,  // drawn with alpha blended antialiasing
      float thickness = 1.0f, // set to >1 for bolder text
      FontHeightType fontHeightType = FONT_ASCENT
   ) const;

   // Sets a "clipping rectangle" by calling glScissor().
   // The client must also call glEnable( GL_SCISSOR_TEST )
   // to activate clipping.
   void setScissorRect( int x, int y, int w, int h ) const;
};


#endif /* DRAWUTIL2D_H */

