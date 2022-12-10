
#ifndef GLOBAL_H
#define GLOBAL_H


#include <stdio.h>   /* printf() */
#include <stdlib.h>  /* exit() */
#ifdef _WIN32
#define M_PI 3.1415926535897932384626433832795
#define M_SQRT2 1.4142135623730950488016887242097
#endif


#ifdef _WIN32
// On linux, I define this in a Makefile.
// On Windows, I like it defined by default.
#define DEBUG
#endif

#ifdef DEBUG
   #define ASSERT(exp) { if (!(exp)) { \
      printf( "Assertion error at line %d in %s\n", __LINE__, __FILE__ ); \
      /*sprintf(0, "This should cause a crash blah blah blah blah.");*/ \
      exit( 1 ); \
   }}
   #define ASSERT_IS_EQUAL(a,b) { \
      ASSERT( (b) - 0.0005 < (a) && (a) < (b) + 0.0005 ); \
   }
#else
   #define ASSERT(exp)
   #define ASSERT_IS_EQUAL(a,b)
#endif


inline int ROUND( float x ) {
   return x < 0 ? -(int)(-x+0.5f) : (int)(x+0.5f);
}
inline int ROUND( double x ) {
   return x < 0 ? -(int)(-x+0.5f) : (int)(x+0.5f);
}


#endif /* GLOBAL_H */

