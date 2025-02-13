//
// Diamond box code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// Box drawing code for an obscure box type.
// These box types are in separate files so they are not linked
// in if not used.

// The diamond box draws best if the area is square!

#include "../hdr/Fl.h"
#include "../hdr/fl_draw.h"

extern const uchar* fl_gray_ramp();

static void fl_diamond_up_box(int x,int y,int w,int h,Fl_Color bgcolor) {
  w &= -2;
  h &= -2;
  int x1 = x+w/2;
  int y1 = y+h/2;
  Fl::set_box_color(bgcolor);
  fl_polygon(x+3, y1, x1,y+3, x+w-3,y1, x1,y+h-3);
  const uchar *g = fl_gray_ramp();
  fl_color(g[(int)'W']); fl_line(x+1, y1, x1, y+1, x+w-1, y1);
  fl_color(g[(int)'U']); fl_line(x+2, y1, x1, y+2, x+w-2, y1);
  fl_color(g[(int)'S']); fl_line(x+3, y1, x1, y+3, x+w-3, y1);
  fl_color(g[(int)'P']); fl_line(x+3, y1, x1, y+h-3, x+w-3, y1);
  fl_color(g[(int)'N']); fl_line(x+2, y1, x1, y+h-2, x+w-2, y1);
  fl_color(g[(int)'H']); fl_line(x+1, y1, x1, y+h-1, x+w-1, y1);
  fl_color(g[(int)'A']); fl_loop(x, y1, x1, y, x+w, y1, x1, y+h);
}

static void fl_diamond_down_box(int x,int y,int w,int h,Fl_Color bgcolor) {
  w &= -2;
  h &= -2;
  int x1 = x+w/2;
  int y1 = y+h/2;
  const uchar *g = fl_gray_ramp();
  fl_color(g[(int)'P']); fl_line(x+0, y1, x1, y+0, x+w-0, y1);
  fl_color(g[(int)'N']); fl_line(x+1, y1, x1, y+1, x+w-1, y1);
  fl_color(g[(int)'H']); fl_line(x+2, y1, x1, y+2, x+w-2, y1);
  fl_color(g[(int)'W']); fl_line(x+2, y1, x1, y+h-2, x+w-2, y1);
  fl_color(g[(int)'U']); fl_line(x+1, y1, x1, y+h-1, x+w-1, y1);
  fl_color(g[(int)'S']); fl_line(x+0, y1, x1, y+h-0, x+w-0, y1);
  Fl::set_box_color(bgcolor);
  fl_polygon(x+3, y1, x1,y+3, x+w-3,y1, x1,y+h-3);
  fl_color(g[(int)'A']); fl_loop(x+3, y1, x1, y+3, x+w-3, y1, x1, y+h-3);
}

void fl_diamond_focus(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg) {
  w &= -2;
  h &= -2;
  x += Fl::box_dx(bt)+4;
  y += Fl::box_dy(bt)+4;
  w -= Fl::box_dw(bt)+8;
  h -= Fl::box_dh(bt)+8;
  int x1 = x+w/2;
  int y1 = y+h/2;
  Fl_Color savecolor = fl_color();
  fl_color(fl_contrast(fg, bg));
  fl_line_style(FL_DOT);
  fl_loop(x,y1, x1,y, x+w,y1, x1,y+h);
  fl_line_style(FL_SOLID);
  fl_color(savecolor);
}
extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*, Fl_Box_Draw_Focus_F* =NULL);
Fl_Boxtype fl_define_FL_DIAMOND_BOX() {
  fl_internal_boxtype(_FL_DIAMOND_DOWN_BOX, fl_diamond_down_box, fl_diamond_focus);
  fl_internal_boxtype(_FL_DIAMOND_UP_BOX, fl_diamond_up_box, fl_diamond_focus);
  return _FL_DIAMOND_UP_BOX;
}
