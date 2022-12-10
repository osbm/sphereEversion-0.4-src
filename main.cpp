
/*
   This file is part of a program called sphereEversion.
   The complete source code can be downloaded from
      http://www.dgp.toronto.edu/~mjmcguff/eversion/

   This file was written by Michael McGuffin, 1998,
   and has undergone slight modifications since then.

   Update: in 2005, this code was ported to GLUT.
   The code is currently somewhat of a hodgepodge, as different parts of
   it were written at different times and with different coding styles.
*/

#include "generateGeometry.h"
#include "Camera.h"
#include "drawutil.h"
#include "drawutil2D.h"

#include <GL/glut.h>
#include <cstring>

Camera * camera = 0;


enum RenderingStyle {
   style_points = 0,
   style_wireframe,
   style_polygons,
   style_checkered,
   style_bands,
   number_of_styles
} renderingStyle = style_polygons;
bool isShadingSmooth = true;
bool useAlphaBlending = false;
float alpha = 0.3f;
bool cullBackfaces = false;
bool flipFrontFaces = true;
bool drawWorldAxes = true;
bool drawTarget = false;
bool displayText = true;
double deltaTime = 1.0/256;
bool showHalfStrips = false;
const int defaultNumStrips = 8;
const int defaultNumberOfLatitudinalPatchesPerHemisphere = 12;
const int defaultNumberOfLongitudinalPatchesPerStrip = 12;
bool isAnythingAnimating = false;
bool animatingEversion = false;
bool animatingEversionBackwards = false;
bool animatingRotation = false;
int timerInterval = 100;  // in milliseconds
Point3 materialColour(1,0,0); // RGB values stored in the x,y,z components
bool rootWindowMode = false;

bool LMB=false, MMB=false, RMB=false;
bool CTRL=false, ALT=false, SHIFT=false;
int mouse_x, mouse_y;

// MI stands for Menu Item
#define MI_CYCLE_DRAW_MODE 11
#define MI_TOGGLE_SMOOTH_SHADING 12
#define MI_TOGGLE_ALPHA_BLENDING 13
#define MI_TOGGLE_DISPLAY_OF_BACKFACES 14
#define MI_TOGGLE_WHICH_FACES_ARE_FRONT_FACING 15
#define MI_TOGGLE_DISPLAY_OF_WORLD_SPACE_AXES 21
#define MI_TOGGLE_DISPLAY_OF_CAMERA_TARGET 22
#define MI_TOGGLE_DISPLAY_OF_TEXT 23
#define MI_INCREMENT_TIME 31
#define MI_DECREMENT_TIME 32
#define MI_DOUBLE_TIME_STEP 33
#define MI_HALVE_TIME_STEP 34
#define MI_INCREMENT_NUM_STRIPS 41
#define MI_DECREMENT_NUM_STRIPS 42
#define MI_INCREMENT_NUM_STRIPS_TO_DISPLAY 43
#define MI_DECREMENT_NUM_STRIPS_TO_DISPLAY 44
#define MI_CHANGE_NUM_HEMISPHERES_TO_DISPLAY 45
#define MI_TOGGLE_HALFSTRIPS 46
#define MI_INCREMENT_LATITUDINAL_RESOLUTION 51
#define MI_DECREMENT_LATITUDINAL_RESOLUTION 52
#define MI_INCREMENT_LONGITUDINAL_RESOLUTION 53
#define MI_DECREMENT_LONGITUDINAL_RESOLUTION 54
#define MI_TOGGLE_ANIMATED_EVERSION 61
#define MI_TOGGLE_ANIMATED_ROTATION 62
#define MI_RESET_CAMERA 71
#define MI_QUIT 81


// forward declarations
void startAnimationAsNecessary();
void timerCallback( int id );


class EvertableSphere {
    double Time;   // between 0.0 and 1.0

    static const int NumHemispheres;
    int NumStrips;
    int NumHemispheresToDisplay;
    int NumStripsToDisplay;
    int NumberOfLatitudinalPatchesPerHemisphere,
        NumberOfLongitudinalPatchesPerStrip;

    // Stores all the vertices and normals used to render the sphere.
    // Elements in the array are arranged by [latitude][longitude].
    GLPoint ** arrayOfVertices;
    bool verticesAreDirty; // If true, need to regenerate vertices.

    void GenerateVertices();
    void DeallocateArray();
public:
    EvertableSphere() : arrayOfVertices(NULL), verticesAreDirty(true) {
       Construct();
    }
    ~EvertableSphere() { DeallocateArray(); verticesAreDirty = true; }

    void Construct(
       double time = 0,
       int numStrips = 0,
       int numHemispheresToDisplay = 0,
       int numStripsToDisplay = 0,
       int numberOfLatitudinalPatchesPerHemisphere = 0,
       int numberOfLongitudinalPatchesPerStrip = 0
    );
    void Reconstruct() { DeallocateArray(); verticesAreDirty = true; }
    void Draw();
    void IncrementTime(double deltaTime) {
       if ( Time < 1.0 ) {
          DeallocateArray();
          Time += deltaTime;
          if ( deltaTime > 1.0 ) deltaTime = 1.0;
          verticesAreDirty = true;
       }
    }
    void DecrementTime(double deltaTime) {
       if ( Time > 0.0 ) {
          DeallocateArray();
          Time -= deltaTime;
          if ( deltaTime < 0.0 ) deltaTime = 0.0;
          verticesAreDirty = true;
       }
    }
    void IncrementNumStrips() {
       DeallocateArray();
       ++ NumStrips;
       ++ NumStripsToDisplay;
       verticesAreDirty = true;
    }
    void DecrementNumStrips() {
       if ( NumStrips > 1 ) {
          DeallocateArray();
          -- NumStrips;
          if ( NumStripsToDisplay > 1 )
             -- NumStripsToDisplay;
          verticesAreDirty = true;
       }
    }
    void ChangeNumHemispheresToDisplay() {
       if ( NumHemispheresToDisplay == NumHemispheres )
          NumHemispheresToDisplay = 1;
       else
          ++ NumHemispheresToDisplay;
    }
    void IncrementNumStripsToDisplay() {
       if ( NumStripsToDisplay < NumStrips ) ++ NumStripsToDisplay;
    }
    void DecrementNumStripsToDisplay() {
       if ( NumStripsToDisplay > 1 ) -- NumStripsToDisplay;
    }
    void IncrementLatitudinalResolution() {
       DeallocateArray();
       ++ NumberOfLatitudinalPatchesPerHemisphere;
       verticesAreDirty = true;
    }
    void DecrementLatitudinalResolution() {
       DeallocateArray();
       -- NumberOfLatitudinalPatchesPerHemisphere;
       verticesAreDirty = true;
    }
    void IncrementLongitudinalResolution() {
       DeallocateArray();
       ++ NumberOfLongitudinalPatchesPerStrip;
       verticesAreDirty = true;
    }
    void DecrementLongitudinalResolution() {
       DeallocateArray();
       -- NumberOfLongitudinalPatchesPerStrip;
       verticesAreDirty = true;
    }

    double GetTime() { return Time; }
};

const int EvertableSphere::NumHemispheres = 2;

void EvertableSphere::DeallocateArray() {

   int j;

   if (arrayOfVertices == NULL)
     return;
   for (j = NumberOfLatitudinalPatchesPerHemisphere; j >= 0; --j)
     delete [] (arrayOfVertices[j]);
   delete [] arrayOfVertices;
   arrayOfVertices = NULL;
}

void EvertableSphere::Construct(
   double time,
   int numStrips,
   int numHemispheresToDisplay,
   int numStripsToDisplay,
   int numberOfLatitudinalPatchesPerHemisphere,
   int numberOfLongitudinalPatchesPerStrip
) {
    if (time < 0.0) Time = 0.0;
    else if (time > 1.0) Time = 1.0;
    else Time = time;

    NumStrips = numStrips < 1 ? defaultNumStrips : numStrips;

    NumHemispheresToDisplay = numHemispheresToDisplay < 1
       ? NumHemispheres : numHemispheresToDisplay;

    NumStripsToDisplay = numStripsToDisplay < 1
       ? NumStrips : numStripsToDisplay;

    if ( numberOfLatitudinalPatchesPerHemisphere <= 0 )
       NumberOfLatitudinalPatchesPerHemisphere
          = defaultNumberOfLatitudinalPatchesPerHemisphere;
    else
       NumberOfLatitudinalPatchesPerHemisphere
          = numberOfLatitudinalPatchesPerHemisphere;

    if ( numberOfLongitudinalPatchesPerStrip <= 0 )
       NumberOfLongitudinalPatchesPerStrip
          = defaultNumberOfLongitudinalPatchesPerStrip;
    else
       NumberOfLongitudinalPatchesPerStrip
          = numberOfLongitudinalPatchesPerStrip;

    verticesAreDirty = true;
}

void EvertableSphere::GenerateVertices() {

    int j;

    // clamp input parameters to their minima
    if (Time < 0.0)
       Time = 0.0;
    else if (Time > 1.0)
       Time = 1.0;

    if (NumberOfLatitudinalPatchesPerHemisphere < 2)
       NumberOfLatitudinalPatchesPerHemisphere = 2;

    if (NumberOfLongitudinalPatchesPerStrip < 2)
       NumberOfLongitudinalPatchesPerStrip = 2;

    // throw away previous geometry
    DeallocateArray();

    // allocate stuff
    arrayOfVertices = new GLPointPointer[1 + NumberOfLatitudinalPatchesPerHemisphere];
    for (j = NumberOfLatitudinalPatchesPerHemisphere; j >= 0; --j)
       arrayOfVertices[j] = new GLPoint[1 + NumberOfLongitudinalPatchesPerStrip];

    // generate the geometry
    generateGeometry(
       arrayOfVertices,
       Time,
       NumStrips,

       0.0,
       NumberOfLatitudinalPatchesPerHemisphere,
       1.0,
       0.0,
       NumberOfLongitudinalPatchesPerStrip,
       showHalfStrips ? 0.5 : 1.0
#ifdef BEND_IN /* this will display a cylindar bending into a sphere */
       ,Time
#endif
    );

    verticesAreDirty = false;
}

void EvertableSphere::Draw() {

   if ( verticesAreDirty ) {
      DeallocateArray();
      GenerateVertices();
      ASSERT( ! verticesAreDirty );
   }

   /*
   In a GL_TRIANGLE_STRIP block, if the vertices sent
   in are numbered 0,1,2,3,... the resulting strip will look like

      0---2---4---6---8--- ...
       \ / \ / \ / \ / \
        1---3---5---7---9 ...

   and will be cowposed of triangles (0,1,2), (2,1,3), (2,3,4), ...
   The ordering is important.  By convention, polygons whose vertices
   appear in counterclockwise order on the screen are front-facing.
   If you create polygons with vertices in the wrong order, and
   back-face culling is turned on, you'll never see the polygons
   appear on the screen.
   */

   int hemisphere,strip,j,k;

   glFrontFace(GL_CW);  // we're going to use the opposite convention

   glMatrixMode(GL_MODELVIEW);

   for (hemisphere = 0; hemisphere < NumHemispheresToDisplay; ++hemisphere) {
      glPushMatrix();
      glRotatef(hemisphere*180.0,0.0,1.0,0.0);
      for (strip = 0; strip < NumStripsToDisplay; ++strip) {
         glPushMatrix();
         glRotatef(
            (hemisphere == 0 ? -strip : strip+1)*360.0/NumStrips,
            0.0,0.0,1.0
         );

         if ( renderingStyle == style_points ) {
            glBegin(GL_POINTS);
            for (j = 0; j <= NumberOfLatitudinalPatchesPerHemisphere; ++j)
               for (k = 0; k <= NumberOfLongitudinalPatchesPerStrip; ++k) {
                  glNormal3fv(arrayOfVertices[j][k].normal);
                  glVertex3fv(arrayOfVertices[j][k].vertex);
               }
            glEnd();
         }
         else {
            for (j = 0; j < NumberOfLatitudinalPatchesPerHemisphere; ++j) {
               if (
                  renderingStyle == style_polygons
                  || renderingStyle == style_wireframe
                  || (renderingStyle == style_bands && (j & 1)==hemisphere)
               ) {
                  glBegin(GL_TRIANGLE_STRIP);
                  for (k = 0; k <= NumberOfLongitudinalPatchesPerStrip; ++k) {
                     glNormal3fv(arrayOfVertices[j][k].normal);
                     glVertex3fv(arrayOfVertices[j][k].vertex);
                     glNormal3fv(arrayOfVertices[j+1][k].normal);
                     glVertex3fv(arrayOfVertices[j+1][k].vertex);
                  }
                  glEnd();
               }
               else if (renderingStyle == style_checkered) {
                  for (k = j%2; k < NumberOfLongitudinalPatchesPerStrip; k+=2) {
                     glBegin(GL_TRIANGLE_STRIP);
                     glNormal3fv(arrayOfVertices[j][k].normal);
                     glVertex3fv(arrayOfVertices[j][k].vertex);
                     glNormal3fv(arrayOfVertices[j+1][k].normal);
                     glVertex3fv(arrayOfVertices[j+1][k].vertex);
                     glNormal3fv(arrayOfVertices[j][k+1].normal);
                     glVertex3fv(arrayOfVertices[j][k+1].vertex);
                     glNormal3fv(arrayOfVertices[j+1][k+1].normal);
                     glVertex3fv(arrayOfVertices[j+1][k+1].vertex);
                     glEnd();
                  }
               }
            }
         }
         glPopMatrix();
      }
      glPopMatrix();
   }
}

// ===============================================================

EvertableSphere sphere;

// ===============================================================

void reshapeCallback( int width, int height ) {
   if ( 0 != camera )
      camera->resizeViewport( width, height );
}

void drawCallback() {

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glDepthFunc( GL_LEQUAL );
   glEnable( GL_DEPTH_TEST );

   camera->transform();

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // ----- draw world space axes

   if ( drawWorldAxes )
      drawAxes( Point3(-1,-1,-1), 1, true );

   // ----- draw little crosshairs at the camera's target

   if ( drawTarget ) {
      const Point3& t = camera->getTarget();
      glColor3f( 0, 1, 1 );
      drawCrossHairs( t, camera->convertLength( t, 0.05 ) );
   }

   // ----- setup for drawing objects

   if (cullBackfaces) {
      // turn on back face culling
      glEnable( GL_CULL_FACE );
      glCullFace( flipFrontFaces ? GL_BACK : GL_FRONT );
   }
   else glDisable( GL_CULL_FACE );

   // Don't bother doing this, since the normals are
   // already normalized by us at generation time.
   //
   // glEnable(GL_NORMALIZE);

   if ( renderingStyle == style_wireframe )
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

   if (
      renderingStyle == style_polygons
      || renderingStyle == style_checkered
      || renderingStyle == style_bands
   ) {
      // Setup colours & lighting properties of the materiel.
      //
      GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
      GLfloat shininess[] = { 50.0 };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
      float a = useAlphaBlending ? alpha : 1;
      GLfloat colourFront[] = {
         a*materialColour.x(),
         a*materialColour.y(),
         a*materialColour.z(),
         1.0
      };// colour of front faces
      GLfloat colourBack[] = {
         a*(1-materialColour.x()),
         a*(1-materialColour.y()),
         a*(1-materialColour.z()),
         1.0
      };// colour of back faces
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colourFront);
      glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, colourBack);

      // Setup lighting.
      //
      // Note that this doesn't seem to properly setup specular lighting.
      // After trying a few unsuccessful modifications to the code,
      // I'm not sure how to fix this.
      // See "man glLight", in the section on GL_POSITION, for information.
      //
      Point3 lightPosition = camera->getPosition()
         + ( camera->getPosition() - camera->getTarget() );
      glLightfv( GL_LIGHT0, GL_POSITION, lightPosition.get() );
      glEnable( GL_LIGHT0 );
      glEnable( GL_LIGHTING );
      GLfloat lightModelFlag[1] = { 1.0 };
      glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lightModelFlag);

      glShadeModel( isShadingSmooth ? GL_SMOOTH : GL_FLAT );
   }

   if ( useAlphaBlending ) {
      // We use an additive (and therefore commutative) function,
      // so that no z-sorting of polygons is necessary.
      glBlendFunc( GL_ONE, GL_ONE );

      glEnable( GL_BLEND );
      glDisable( GL_DEPTH_TEST );
   }

   glColor3f( 1, 1, 1 );

   // ----- draw objects

   sphere.Draw();

   if ( renderingStyle == style_wireframe )
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   glDisable( GL_LIGHTING );
   if ( useAlphaBlending ) {
      glDisable( GL_BLEND );
      glEnable( GL_DEPTH_TEST );
   }
   glDisable( GL_CULL_FACE );

   // ----- draw text

   if ( displayText ) {
      OpenGL2DInterface g;
      g.pushProjection(
         camera->getViewportWidthInPixels(),camera->getViewportHeightInPixels()
      );
      char buffer[50];
      sprintf( buffer, "t = %.4f, delta_t = 1/%d",
         sphere.GetTime(),
         ROUND( 1.0/deltaTime )
      );

      int FONT_HEIGHT = 18;
      g.drawString(
         20, 20+FONT_HEIGHT,
         buffer,
         FONT_HEIGHT,
         true, // blended ?
         1, // line thinkness
         OpenGL2DInterface::FONT_TOTAL_HEIGHT
      );

      if ( useAlphaBlending ) {
         sprintf( buffer, "alpha = %.3f", alpha );
         g.drawString(
            20, 25+2*FONT_HEIGHT,
            buffer,
            FONT_HEIGHT,
            true, // blended ?
            1, // line thinkness
            OpenGL2DInterface::FONT_TOTAL_HEIGHT
         );
      }

      g.popProjection();
   }

   // ----- finish up

   glutSwapBuffers();
}

void mouseCallback( int button, int state, int x, int y ) {
   int mod = glutGetModifiers();
   CTRL = mod & GLUT_ACTIVE_CTRL;
   ALT = mod & GLUT_ACTIVE_ALT;
   SHIFT = mod & GLUT_ACTIVE_SHIFT;

   switch ( button ) {
      case GLUT_LEFT_BUTTON : LMB = state == GLUT_DOWN; break;
      case GLUT_MIDDLE_BUTTON : MMB = state == GLUT_DOWN; break;
      case GLUT_RIGHT_BUTTON : RMB = state == GLUT_DOWN; break;
   }

   mouse_x = x;
   mouse_y = y;

   /*
   printf("buttons:(%d,%d,%d); modifiers:(%c,%c,%c); position:(%d,%d)\n",
      LMB ? 1 : 0, MMB ? 1 : 0, RMB ? 1 : 0,
      CTRL ? 'C' : '-', ALT ? 'A' : '-', SHIFT ? 'S' : '-',
      mouse_x, mouse_y
   );
   */
}

void passiveMotionCallback( int x, int y ) {
   mouse_x = x;
   mouse_y = y;
   // printf("move:(%d,%d)\n", mouse_x, mouse_y );
}

void motionCallback( int x, int y ) {
   int delta_x = x - mouse_x;
   int delta_y = mouse_y - y;
   int delta = delta_x + delta_y;

   if ( SHIFT ) {
      // The redundant functionality here is for users with 2-button mice.
      if ( LMB ) {
         if ( ALT )
            camera->dollyCameraForward( 3*delta, false );
         else
            camera->translateSceneRightAndUp( delta_x, delta_y );
      }
   }
#if 0
   else if ( CTRL ) {
      if ( LMB && MMB )
         camera->rollCameraRight( delta );
      else if ( LMB ) {
         camera->pitchCameraUp( delta_y );
         camera->yawCameraRight( delta_x );
      }
      else if ( MMB )
         camera->zoomIn( delta );
   }
#else
   else if ( CTRL ) {
      if ( LMB ) {
         alpha += 0.005f*delta_x;
         if ( alpha < 0 ) alpha = 0;
         else if ( alpha > 1 ) alpha = 1;
         glutPostRedisplay();
      }
   }
#endif
   else if ( ALT ) {
      if ( LMB && MMB )
         camera->dollyCameraForward( 3*delta, false );
      else if ( LMB )
         camera->orbit( mouse_x, mouse_y, x, y );
      else if ( MMB )
         camera->translateSceneRightAndUp( delta_x, delta_y );
   }
   else {
      if ( delta_x > 0 )
         sphere.IncrementTime(deltaTime*delta_x);
      else
         sphere.DecrementTime(deltaTime*(-delta_x));
   }

   glutPostRedisplay();

   mouse_x = x;
   mouse_y = y;
   // printf("drag:(%d,%d)\n", mouse_x, mouse_y );
}

void menuCallback( int menuItem ) {
   switch ( menuItem ) {

      case MI_CYCLE_DRAW_MODE :
         renderingStyle
            = (RenderingStyle)(((int)renderingStyle + 1) % number_of_styles);
         glutPostRedisplay();
         break;
      case MI_TOGGLE_SMOOTH_SHADING :
         isShadingSmooth = ! isShadingSmooth;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_ALPHA_BLENDING :
         useAlphaBlending = ! useAlphaBlending;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_DISPLAY_OF_BACKFACES :
         cullBackfaces = ! cullBackfaces;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_WHICH_FACES_ARE_FRONT_FACING :
         flipFrontFaces = ! flipFrontFaces;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_DISPLAY_OF_WORLD_SPACE_AXES :
         drawWorldAxes = ! drawWorldAxes;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_DISPLAY_OF_CAMERA_TARGET :
         drawTarget = ! drawTarget;
         glutPostRedisplay();
         break;
      case MI_TOGGLE_DISPLAY_OF_TEXT :
         displayText = ! displayText;
         glutPostRedisplay();
         break;
      case MI_INCREMENT_TIME :
         sphere.IncrementTime(deltaTime);
         glutPostRedisplay();
         break;
      case MI_DECREMENT_TIME :
         sphere.DecrementTime(deltaTime);
         glutPostRedisplay();
         break;
      case MI_DOUBLE_TIME_STEP :
         deltaTime *= 2.0;
         if (deltaTime > 1.0)
            deltaTime = 1.0;
         glutPostRedisplay();
         break;
      case MI_HALVE_TIME_STEP :
         if (deltaTime * 0.5 >= 1.0/8192)
            deltaTime *= 0.5;
         glutPostRedisplay();
         break;
      case MI_INCREMENT_NUM_STRIPS :
         sphere.IncrementNumStrips();
         glutPostRedisplay();
         break;
      case MI_DECREMENT_NUM_STRIPS :
         sphere.DecrementNumStrips();
         glutPostRedisplay();
         break;
      case MI_INCREMENT_NUM_STRIPS_TO_DISPLAY :
         sphere.IncrementNumStripsToDisplay();
         glutPostRedisplay();
         break;
      case MI_DECREMENT_NUM_STRIPS_TO_DISPLAY :
         sphere.DecrementNumStripsToDisplay();
         glutPostRedisplay();
         break;
      case MI_CHANGE_NUM_HEMISPHERES_TO_DISPLAY :
         sphere.ChangeNumHemispheresToDisplay();
         glutPostRedisplay();
         break;
      case MI_TOGGLE_HALFSTRIPS :
         showHalfStrips = ! showHalfStrips;
         sphere.Reconstruct();
         glutPostRedisplay();
         break;
      case MI_INCREMENT_LATITUDINAL_RESOLUTION :
         sphere.IncrementLatitudinalResolution();
         glutPostRedisplay();
         break;
      case MI_DECREMENT_LATITUDINAL_RESOLUTION :
         sphere.DecrementLatitudinalResolution();
         glutPostRedisplay();
         break;
      case MI_INCREMENT_LONGITUDINAL_RESOLUTION :
         sphere.IncrementLongitudinalResolution();
         glutPostRedisplay();
         break;
      case MI_DECREMENT_LONGITUDINAL_RESOLUTION :
         sphere.DecrementLongitudinalResolution();
         glutPostRedisplay();
         break;
      case MI_TOGGLE_ANIMATED_EVERSION :
         animatingEversion = ! animatingEversion;
         startAnimationAsNecessary();
         break;
      case MI_TOGGLE_ANIMATED_ROTATION :
         animatingRotation = ! animatingRotation;
         startAnimationAsNecessary();
         break;
      case MI_RESET_CAMERA :
         camera->reset();
         glutPostRedisplay();
         break;
      case MI_QUIT :
         exit(0);
         break;
      default:
         printf("unknown menu item %d was selected\n", menuItem );
         break;
   }
}

void keyboardCallback( unsigned char key, int x, int y ) {
   switch ( key ) {
      case ' ':
         menuCallback( MI_CYCLE_DRAW_MODE );
         break;
      case '+':
         menuCallback( MI_INCREMENT_TIME );
         break;
      case '-':
         menuCallback( MI_DECREMENT_TIME );
         break;
      case '*':
         menuCallback( MI_DOUBLE_TIME_STEP );
         break;
      case '/':
         menuCallback( MI_HALVE_TIME_STEP );
         break;
      case 'a':
         menuCallback( MI_TOGGLE_ALPHA_BLENDING );
         break;
      case 'b':
         menuCallback( MI_TOGGLE_DISPLAY_OF_BACKFACES );
         break;
      case 'f':
         menuCallback( MI_TOGGLE_WHICH_FACES_ARE_FRONT_FACING );
         break;
      case 'r':
         menuCallback( MI_RESET_CAMERA );
         break;
      case 's':
         menuCallback( MI_TOGGLE_SMOOTH_SHADING );
         break;
      case 't':
         menuCallback( MI_TOGGLE_DISPLAY_OF_CAMERA_TARGET );
         break;
      case 'w':
         menuCallback( MI_TOGGLE_DISPLAY_OF_WORLD_SPACE_AXES );
         break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
         materialColour.x() = (key-'1') & 1;
         materialColour.y() = ((key-'1')>>1) & 1;
         materialColour.z() = ((key-'1')>>2) & 1;
         glutPostRedisplay();
         break;
      case 27: // Escape
         menuCallback( MI_QUIT );
         break;
      default:
         printf("untrapped key %d\n", (int)key );
         break;
   }
}

void specialCallback( int key, int x, int y ) {
   switch ( key ) {
      // function keys
      case GLUT_KEY_F1 :
         menuCallback( MI_INCREMENT_LATITUDINAL_RESOLUTION );
         break;
      case GLUT_KEY_F2 :
         menuCallback( MI_DECREMENT_LATITUDINAL_RESOLUTION );
         break;
      case GLUT_KEY_F3 :
         menuCallback( MI_INCREMENT_LONGITUDINAL_RESOLUTION );
         break;
      case GLUT_KEY_F4 :
         menuCallback( MI_DECREMENT_LONGITUDINAL_RESOLUTION );
         break;
      case GLUT_KEY_F5 :
         menuCallback( MI_TOGGLE_ANIMATED_EVERSION );
         break;
      case GLUT_KEY_F6 :
         menuCallback( MI_TOGGLE_ANIMATED_ROTATION );
         break;
      case GLUT_KEY_F9 :
         menuCallback( MI_TOGGLE_DISPLAY_OF_TEXT );
         break;

      // arrow keys
      case GLUT_KEY_LEFT :
         menuCallback( MI_DECREMENT_TIME );
         break;
      case GLUT_KEY_RIGHT :
         menuCallback( MI_INCREMENT_TIME );
         break;
      case GLUT_KEY_UP :
         menuCallback( MI_INCREMENT_NUM_STRIPS_TO_DISPLAY );
         break;
      case GLUT_KEY_DOWN :
         menuCallback( MI_DECREMENT_NUM_STRIPS_TO_DISPLAY );
         break;


      case GLUT_KEY_PAGE_UP:
         menuCallback( MI_INCREMENT_NUM_STRIPS );
         break;
      case GLUT_KEY_PAGE_DOWN:
         menuCallback( MI_DECREMENT_NUM_STRIPS );
         break;
      case GLUT_KEY_HOME:
         menuCallback( MI_TOGGLE_HALFSTRIPS );
         break;
      case GLUT_KEY_END:
         menuCallback( MI_CHANGE_NUM_HEMISPHERES_TO_DISPLAY );
         break;


      default:
         printf("untrapped special key %d\n", (int)key );
         break;
   }
}

void startAnimationAsNecessary() {
   if ( ! isAnythingAnimating ) {
      if ( animatingEversion || animatingRotation ) {
         isAnythingAnimating = true;
         glutTimerFunc( timerInterval, timerCallback, 0 );
      }
   }
}

void timerCallback( int /* id */ ) {

   // puts("timer callback called!");

   if ( animatingEversion || animatingRotation ) {
      if ( animatingEversion ) {
         // Do not evert if the user is currently trying to evert
         if ( !LMB || ( MMB || RMB || CTRL || ALT || SHIFT ) ) {
            if (animatingEversionBackwards) {
               if (sphere.GetTime() == 0.0)
                  animatingEversionBackwards = ! animatingEversionBackwards;
               else
                  sphere.DecrementTime(deltaTime);
            }
            else {
               if (sphere.GetTime() == 1.0)
                  animatingEversionBackwards = ! animatingEversionBackwards;
               else
                  sphere.IncrementTime(deltaTime);
            }
         }
      }

      if ( animatingRotation ) {
         // Do not rotate if the user is currently trying to rotate
         if ( !LMB || ( MMB || RMB || CTRL || !ALT || SHIFT ) ) {
            int x = camera->getViewportWidthInPixels()/2;
            int y = camera->getViewportHeightInPixels()/2;
            camera->orbit( x, y, x+0.5f, y+0.5f );
         }
      }

      glutPostRedisplay();
      glutTimerFunc( timerInterval, timerCallback, 0 );
   }
   else {
      isAnythingAnimating = false;
      // puts("stop");
   }
}

int main( int argc, char *argv[] ) {

   if ( argc > 2 || (argc == 2 && strcmp(argv[1],"--root")!= 0)) {
      fprintf(stderr,"Usage: %s [--root]\n", argv[0] );
      exit(1);
   }
   if ( argc == 2 ) {
      animatingEversion = true;
      animatingRotation = true;
      displayText = false;
      drawWorldAxes = false;
      rootWindowMode = true;
   }

   glutInit( &argc, argv );
   glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
   glutInitWindowSize( 512, 512 );
   glutCreateWindow("Sphere Eversion");

   glutDisplayFunc( drawCallback );
   glutReshapeFunc( reshapeCallback );
   glutKeyboardFunc( keyboardCallback );
   glutSpecialFunc( specialCallback );
   glutMouseFunc( mouseCallback );
   glutPassiveMotionFunc( passiveMotionCallback );
   glutMotionFunc( motionCallback );

   // This is necessary for the --root flag to work.
   startAnimationAsNecessary();

   // create a menu
   int mainMenu = glutCreateMenu( menuCallback );
   glutSetMenu( mainMenu ); // make it the current menu
   glutAddMenuEntry( "Cycle Display Mode (space)", MI_CYCLE_DRAW_MODE );
   glutAddMenuEntry( "Toggle Smooth Shading (s)", MI_TOGGLE_SMOOTH_SHADING );
   glutAddMenuEntry( "Toggle Alpha Blending (a)", MI_TOGGLE_ALPHA_BLENDING );
   glutAddMenuEntry( "Toggle Backface Culling (b)",
      MI_TOGGLE_DISPLAY_OF_BACKFACES );
   glutAddMenuEntry( "Toggle Which Faces Are Front-Facing (f)",
      MI_TOGGLE_WHICH_FACES_ARE_FRONT_FACING );
   glutAddMenuEntry( "Toggle Display of World Space Axes (w)",
      MI_TOGGLE_DISPLAY_OF_WORLD_SPACE_AXES );
   glutAddMenuEntry( "Toggle Display of Camera Target (t)",
      MI_TOGGLE_DISPLAY_OF_CAMERA_TARGET );
   glutAddMenuEntry( "Toggle Display of Text (F9)",
      MI_TOGGLE_DISPLAY_OF_TEXT );
   glutAddMenuEntry( "Increment Time t (+ or right arrow)",
      MI_INCREMENT_TIME );
   glutAddMenuEntry( "Decrement Time t (- or left arrow)",
      MI_DECREMENT_TIME );
   glutAddMenuEntry( "Double Time Step delta_t (*)", MI_DOUBLE_TIME_STEP );
   glutAddMenuEntry( "Halve Time Step delta_t (/)", MI_HALVE_TIME_STEP );
   glutAddMenuEntry( "Increment Total Number of Strips (page up)",
      MI_INCREMENT_NUM_STRIPS );
   glutAddMenuEntry( "Decrement Total Number of Strips (page down)",
      MI_DECREMENT_NUM_STRIPS );
   glutAddMenuEntry( "Increment Number of Strips Displayed (up arrow)",
      MI_INCREMENT_NUM_STRIPS_TO_DISPLAY );
   glutAddMenuEntry( "Decrement Number of Strips Displayed (down arrow)",
      MI_DECREMENT_NUM_STRIPS_TO_DISPLAY );
   glutAddMenuEntry( "Toggle Display of One Hemisphere (end)",
      MI_CHANGE_NUM_HEMISPHERES_TO_DISPLAY );
   glutAddMenuEntry( "Toggle Display of Half-strips (home)",
      MI_TOGGLE_HALFSTRIPS );
   glutAddMenuEntry( "Increment Number of Latitudinal Patches (F1)",
      MI_INCREMENT_LATITUDINAL_RESOLUTION );
   glutAddMenuEntry( "Decrement Number of Latitudinal Patches (F2)",
      MI_DECREMENT_LATITUDINAL_RESOLUTION );
   glutAddMenuEntry( "Increment Number of Longitudinal Patches (F3)",
      MI_INCREMENT_LONGITUDINAL_RESOLUTION );
   glutAddMenuEntry( "Decrement Number of Longitudinal Patches (F4)",
      MI_DECREMENT_LONGITUDINAL_RESOLUTION );
   glutAddMenuEntry( "Toggle Animated Eversion (F5)",
      MI_TOGGLE_ANIMATED_EVERSION );
   glutAddMenuEntry( "Toggle Animated Rotation (F6)",
      MI_TOGGLE_ANIMATED_ROTATION );
   glutAddMenuEntry( "Reset Camera (r)", MI_RESET_CAMERA );
   glutAddMenuEntry( "Quit (Esc)", MI_QUIT );
   glutAttachMenu( GLUT_RIGHT_BUTTON );//attach the menu to the current window

   int width = glutGet( GLUT_WINDOW_WIDTH );
   int height = glutGet( GLUT_WINDOW_HEIGHT );
   camera = new Camera(
      width, height, 2, Point3( 0, 0, 0)
   );

   OpenGL2DInterface::setUseTextureFont( true );

   glutMainLoop();
   return 0;
}

