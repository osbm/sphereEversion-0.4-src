
// GL Points are points on the surface of an object
// to be rendered.  They have a location (x,y,z) and a
// normal vector (nx,ny,nz).  Axis conventions:
// x+ right, y+ up, z+ out of the screen

struct GLPoint {

    // Coordinates stored in arrays are convenient for passing into OpenGL calls
    float vertex[3],   // contains location (x,y,z)
          normal[3];   // contains normal vector (nx,ny,nz)
};

typedef GLPoint * GLPointPointer;

// ----------------------------------------

void generateGeometry(
   GLPoint ** geometryMatrix,   // Must be an array of (1 + u_count) arrays of (1 + v_count) elements
   double time = 0.0,   // must be between 0.0 and 1.0
   int numStrips = 8,   // If this is too small, there will be pinches in the eversion.

   // The u parameter corresponds to latitude.
   // Parallels [90 degrees north, 90 degrees south] are mapped to [0.0, 2.0].
   //
   double u_min = 0.0,   // Set to 0.0 to start at north pole
   int u_count = 12,     // Recommended value: 12*(u_max-u_min)
   double u_max = 1.0,   // Set to 1.0 to stop at equator, or 2.0 to stop at south pole

   // The v parameter corresponds to longitude.
   // Meridians [0 degrees, 180 degrees west] are mapped to [0, numStrips].
   //
   double v_min = 0.0,
   int v_count = 12,     // Recommended value: 12*(v_max-v_min)
   double v_max = 1.0,


   double bendtime = -1.0,   // -1 means don't do bendtime at all

   double corrStart   = 0.00,   // start of corrugation
   double pushStart   = 0.10,   // start of push (poles are pushed through each other)
   double twistStart  = 0.23,   // start of twist (poles rotate in opposite directions)
   double unpushStart = 0.60,   // start of unpush (poles held fixed while corrugations pushed through center)
   double uncorrStart = 0.93    // start of uncorrugation
);
