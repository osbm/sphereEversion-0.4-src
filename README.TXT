
This program displays a sphere undergoing the Thurston eversion.
Hold the left mouse button and drag horizontally to evert the sphere.
Hold down the ALT key and then the left mouse button, and drag to orbit.

MOUSE ACTIONS
  left button            : Evert the sphere by dragging horizontally.
  ALT+left button        : Orbit, i.e. tumble the view.
  ALT+middle button      : Pan, i.e. track the view.
                           If you don't have a middle mouse button,
                           use SHIFT+left button.
  ALT+left+middle buttons: Dolly, i.e. translate the view in-out.
                           If you don't have a middle mouse button,
                           use ALT+SHIFT+left button.
  CTRL+left button       : Change the alpha value by dragging horizontally.
                           This only works if alpha blending is enabled.
  right button           : Popup a menu of options.

KEYBOARD ACTIONS
  left,right arrows
  OR -,+          : Decrease,increase time t by time step delta_t.
                    This is provides an alternative way of everting
                    the sphere, instead of dragging with the mouse.
  *,/             : Multiply,divide time step delta_t by 2.
                    This changes the speed at which the eversion is performed.
  space           : Cycle through different rendering styles
  s               : Toggle smooth/flat shading
  a               : Toggle alpha blending
  b               : Toggle backface culling
  f               : Toggle which faces are front facing
  w               : Toggle display of world space axes
  t               : Toggle display of camera target point
  F9              : Toggle display of text
  page up,down    : Increase,decrease total number of strips
  up,down arrows  : Increase,decrease number of strips displayed
  end             : Toggle display of one hemisphere
  home            : Toggle display of half-strips
  F1,F2           : Increase,decrease number of latitudinal patches
  F3,F4           : Increase,decrease number of longitudinal patches
  F5,F6           : Toggle animated eversion,rotation
  r               : Reset camera
  1-8             : Select colour of faces
  Escape          : Quit

AUXILIARY FILES
  The pre-compiled version of this software comes with a copy
  of glut32.dll, which is necessary for running it on MS Windows.
  You can leave the glut32.dll file in the same directory as the executable,
  or move glut32.dll to C:\winnt\system32\ or C:\windows\system32\
  or just delete it if you already have glut installed in one
  of those directories.

