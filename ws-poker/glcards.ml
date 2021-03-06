open GL
open Sdl
open Loader

let card_width = 512.0;;
let card_height = 715.0;;

let card_aspect = card_width /. card_height;; 

let render c =
  let t = Loader.bind_texture ("cards/" ^ c ^ ".png") in

  glEnable GL_TEXTURE_2D;
  glDisable GL_LIGHTING;

  glEnable GL_CULL_FACE;
  glCullFace GL_BACK;

  glEnable GL_DEPTH_TEST;
  
  glPushMatrix();

  glTranslatev (card_aspect /. (-2.0), -0.5, 0.0);
  
  glBegin GL_QUADS;
  glTexCoord2v (1.0, 1.0); glVertex2v (card_aspect, 1.0);
  glTexCoord2v (1.0, 0.0); glVertex2v (card_aspect, 0.0);
  glTexCoord2v (0.0, 0.0); glVertex2v (0.0, 0.0);
  glTexCoord2v (0.0, 1.0); glVertex2v (0.0, 1.0);
  glEnd ();

  glTranslatev (0.0, 0.0, 0.0);

  ignore (bind_texture ("cards/Red_Back.png"));

  glBegin GL_QUADS;
  glTexCoord2v (1.0, 1.0); glVertex2v (card_aspect, 1.0);
  glTexCoord2v (0.0, 1.0); glVertex2v (0.0, 1.0);
  glTexCoord2v (0.0, 0.0); glVertex2v (0.0, 0.0);
  glTexCoord2v (1.0, 0.0); glVertex2v (card_aspect, 0.0);
  glEnd ();

  glPopMatrix();

  glDisable GL_TEXTURE_2D

