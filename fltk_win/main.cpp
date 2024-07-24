#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include "fltk/hdr/Fl.h"
#include "fltk/hdr/Fl_Window.h"
#include "fltk/hdr/Fl_Box.h"

#include "tree.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Gdiplus.lib")

/**
 // Global callback event counter
*/
static int G_cb_counter = 0;

/**
 Return an Fl_Tree_Reason as a text string name
*/
const char* reason_as_name(Fl_Tree_Reason reason) {
    switch (reason) {
    case FL_TREE_REASON_NONE:       return("none");
    case FL_TREE_REASON_SELECTED:   return("selected");
    case FL_TREE_REASON_DESELECTED: return("deselected");
    case FL_TREE_REASON_OPENED:     return("opened");
    case FL_TREE_REASON_CLOSED:     return("closed");
    case FL_TREE_REASON_DRAGGED:    return("dragged");
    case FL_TREE_REASON_RESELECTED: return("reselected");
    default:                        return("???");
    }
}

void Button_CB(Fl_Widget* w, void* data) {
    // Each push changes height so we can test 'Item h() from widget'
    //    Cycle through larger sizes until 50, then wrap to normal size.
    //
    //    In the case of 'ccc button', it is the child widget, so change
    //    its size.
    //
    //    In the case of the 'D1' and 'D2' buttons, the parent's Fl_Group
    //    is the child, so change the size of that instead.
    //
    Fl_Widget* cw = strcmp(w->label(), "ccc button") == 0 ? w : w->parent();
    int height = cw->h();
    height += 10;
    if (height > 50) height = 20;
    cw->resize(cw->x(), cw->y(), cw->w(), height);
    tree->redraw(); // adjusted height
    tty->printf("'%s' button pushed (height=%d)\n", w->label(), height);
}

/**
 Assign user icons to the items
*/
void AssignUserIcons() {
    static const char* L_folder_xpm[] = {
        "11 11 3 1",
        ".  c None",
        "x  c #d8d833",
        "@  c #808011",
        "...........",
        ".....@@@@..",
        "....@xxxx@.",
        "@@@@@xxxx@@",
        "@xxxxxxxxx@",
        "@xxxxxxxxx@",
        "@xxxxxxxxx@",
        "@xxxxxxxxx@",
        "@xxxxxxxxx@",
        "@xxxxxxxxx@",
        "@@@@@@@@@@@" };
    static Fl_Pixmap L_folder_pixmap(L_folder_xpm);

    static const char* L_document_xpm[] = {
        "11 11 3 1",
        ".  c None",
        "x  c #d8d8f8",
        "@  c #202060",
        ".@@@@@@@@@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@xxxxxxx@.",
        ".@@@@@@@@@." };
    static Fl_Pixmap L_document_pixmap(L_document_xpm);

    // Create deactivated version of document icon
    static Fl_Pixmap L_folder_deicon_pixmap(L_folder_xpm); // copy
    static Fl_Pixmap L_document_deicon_pixmap(L_document_xpm); // copy
    static int first = 1;
    if (first) {
        L_folder_deicon_pixmap.inactive();
        L_document_deicon_pixmap.inactive();
        first = 0;
    }

    // Assign user icons to tree items
    for (Fl_Tree_Item* item = tree->first(); item; item = item->next()) {
        if (usericon_radio->value()) {
            // Assign custom icons
            if (item->has_children()) {
                item->usericon(&L_folder_pixmap);
                item->userdeicon(&L_folder_deicon_pixmap);
            }
            else {
                item->usericon(&L_document_pixmap);
                item->userdeicon(&L_document_deicon_pixmap);
            }
        }
        else {
            // Don't assign custom icons
            item->usericon(0);

            item->userdeicon(0);
        }
    }
    tree->redraw();
}

/**
 Rebuild the 'example tree' from scratch
*/
void RebuildTree() {
    // REBUILD THE TREE TO MAKE CURRENT "DEFAULT" PREFS TAKE EFFECT

    Fl_Tree_Item* proot;
    Fl_Tree_Item* pitem;

    proot = tree->root();

    tree->clear();
    tree->add("Aaa");
    tree->add("Bbb");
    tree->add("Ccc");
    tree->add("Ddd");
    tree->add("Bbb/child-01");
    tree->add("Bbb/child-01/111");
    tree->add("Bbb/child-01/222");
    tree->add("Bbb/child-01/333");
    tree->add("Bbb/child-02");
    tree->add("Bbb/child-03");
    tree->add("Bbb/child-04");

    {
        static Fl_Input* in = 0;
        // Assign an FLTK input widget to one of the items with a label() of its own (STR#2832)
        Fl_Tree_Item* i;
        if ((i = tree->find_item("Bbb/child-02")) != NULL) {
            if (!in) {            // only do this once at program startup
                tree->begin();
                in = new Fl_Input(1, 1, 100, 1, "Fl_Input test");       // we control w() only
                in->labelsize(10);
                in->textsize(10);
                in->align(FL_ALIGN_RIGHT);                          // show label to the right of the widget
                in->tooltip("Fl_Input inside tree.\n"
                    "The widget's label 'Fl_Input test' should appear to the widget's right.");
            }
            in->show();
            i->widget(in);
            tree->end();
        }
    }

    {
        static Fl_Button* but = 0;
        // Assign an FLTK widget to one of the items
        Fl_Tree_Item* i;
        if ((i = tree->find_item("Bbb/child-03")) != NULL) {
            if (!but) {           // only do this once at program startup
                tree->begin();
                but = new Fl_Button(1, 1, 140, 1, "ccc button");     // we control w() only
                but->labelsize(10);
                but->callback(Button_CB);
                but->tooltip("Button inside tree.\n"
                    "If 'Item h() from widget' enabled, "
                    "pressing button enlarges it.");
            }
            but->show();
            i->widget(but);
            tree->end();
        }
    }
    {
        // Assign an FLTK group to one of the items with widgets
        Fl_Tree_Item* i;
        const char* tipmsg = "A group of two buttons inside the tree.\n"
            "If 'Item h() from widget' enabled, "
            "pressing either button enlarges the group "
            "and both buttons together.";
        if ((i = tree->find_item("Bbb/child-04")) != NULL) {
            static Fl_Group* grp = 0;
            if (!grp) {           // only do this once at program startup
                tree->begin();
                grp = new Fl_Group(100, 100, 140, 18); // build group.. tree handles position
                grp->color(FL_WHITE);
                grp->begin();
                Fl_Button* abut = new Fl_Button(grp->x() + 0, grp->y() + 2, 65, 15, "D1");
                abut->labelsize(10);
                abut->callback(Button_CB);
                abut->tooltip(tipmsg);
                Fl_Button* bbut = new Fl_Button(grp->x() + 75, grp->y() + 2, 65, 15, "D2");
                bbut->labelsize(10);
                bbut->callback(Button_CB);
                bbut->tooltip(tipmsg);

                grp->end();
                grp->resizable(grp);
                tree->end();
            }
            grp->show();
            i->widget(grp);
        }
    }

    // Add an 'Ascending' node, and create it sorted
    tree->sortorder(FL_TREE_SORT_NONE);

    pitem = proot->add(tree->prefs(), "Ascending");
    pitem->close();
    //tree->add("Ascending")->close();
    tree->sortorder(FL_TREE_SORT_ASCENDING);
    pitem->add(tree->prefs(), "Zzz");
    pitem->add(tree->prefs(), "Xxx");
    pitem->add(tree->prefs(), "Aaa");
    pitem->add(tree->prefs(), "Bbb");
    pitem->add(tree->prefs(), "Yyy");
    pitem->add(tree->prefs(), "Ccc");
    //tree->add("Ascending/Zzz");
    //tree->add("Ascending/Xxx");
    //tree->add("Ascending/Aaa");
    //tree->add("Ascending/Bbb");
    //tree->add("Ascending/Yyy");
    //tree->add("Ascending/Ccc");

    // Add a 'Descending' node, and create it sorted
    tree->sortorder(FL_TREE_SORT_NONE);
    pitem = proot->add(tree->prefs(), "Descending");
    pitem->close();
    tree->sortorder(FL_TREE_SORT_DESCENDING);
    pitem->add(tree->prefs(), "Zzz");
    pitem->add(tree->prefs(), "Xxx");
    pitem->add(tree->prefs(), "Aaa");
    pitem->add(tree->prefs(), "Bbb");
    pitem->add(tree->prefs(), "Yyy");
    pitem->add(tree->prefs(), "Ccc");
    //tree->add("Descending/Zzz");
    //tree->add("Descending/Xxx");
    //tree->add("Descending/Aaa");
    //tree->add("Descending/Bbb");
    //tree->add("Descending/Yyy");
    //tree->add("Descending/Ccc");

    // Add a long line to trigger horiz scrollbar
    tree->sortorder(FL_TREE_SORT_NONE);
    pitem = proot->add(tree->prefs(), "Long Line");
    pitem->close();
    pitem->add(tree->prefs(), "The quick brown fox jumped over the lazy dog. 0123456789");
    pitem = pitem->add(tree->prefs(), "Longer Line");
    pitem->close();
    pitem->add(tree->prefs(), "The quick brown fox jumped over the lazy dog. ---------------- 0123456789");

    pitem = proot->add(tree->prefs(), "Long Line");
    proot->add(tree->prefs(), (const char *)NULL);
    pitem->labelsize(90);

    pitem = proot->add(tree->prefs(), "500 Items");
    // Add 500 items in numerical order
    for (int t = 0; t < 500; t++) {
        char s[80];
        snprintf(s, 80, "item %04d", t + 1);
        pitem->add(tree->prefs(), s);
    }
    pitem->close();
    //tree->close("500 Items");       // close the 500 items by default

    AssignUserIcons();

    tree->redraw();
    Fl_Group::current(0);
}

/**
 Prompt the user to change the specified color
*/
Fl_Color EditColor(Fl_Color& val) {
    // Returns:
    //     1 if color picked with new color in 'val'
    //     0 if user hit 'Cancel'.
    //
    uchar r, g, b;

    // Get the current color
    Fl::get_color(val, r, g, b);

    // Bring up a color chooser to edit it
    int ret = fl_color_chooser("Choose Color", r, g, b);

    val = fl_rgb_color(r, g, b);


    return(ret);
}

/**
 Updates the color chips with current colors from widget
*/
void UpdateColorChips() {
    color_button->color(tree->color());
    labelcolor_button->color(tree->labelcolor());
    selection_color_button->color(tree->selection_color());

    item_labelfgcolor_button->color(tree->item_labelfgcolor());
    item_labelbgcolor_button->color(tree->item_labelbgcolor());

    all_labelfgcolor_button->color(tree->item_labelfgcolor());  // use default
    all_labelbgcolor_button->color(tree->item_labelbgcolor());  // use default

    window->redraw();
}

/**
 Get the current 'margin bottom' size
 Handles this as an ABI feature..
*/
int GetTreeMarginBottom() {
    return tree->marginbottom();
}

/**
 Get the current 'margin bottom' size
 Handles this as an ABI feature..
*/
int GetTreeWidgetMarginLeft() {
    return tree->widgetmarginleft();
}

/**
 Return the selected item's fg color
*/
Fl_Color GetSelectedItemFGColor() {
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            return(item->labelfgcolor());
        }
    }
    // No items selected? Use default
    return(tree->item_labelfgcolor());
}

/**
 Return the selected item's bg color
*/
Fl_Color GetSelectedItemBGColor() {
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            return(item->labelbgcolor());
        }
    }
    // No items selected? Use default
    return(tree->item_labelbgcolor());
}

Fl_Double_Window* window = (Fl_Double_Window*)0;

Fl_Tree* tree = (Fl_Tree*)0;

static void cb_tree(Fl_Tree*, void*) {
    G_cb_counter++; // Increment callback counter whenever tree callback is invoked
    Fl_Tree_Item* item = tree->callback_item();
    if (item) {
        tty->printf("TREE CALLBACK: label='%s' userdata=%ld reason=%s, changed=%d",
            item->label(),
            (long)(fl_intptr_t)tree->user_data(),
            reason_as_name(tree->callback_reason()),
            tree->changed() ? 1 : 0);
        // More than one click? show click count
        //    Should only happen if reason==FL_TREE_REASON_RESELECTED.
        //
        if (Fl::event_clicks() > 0) {
            tty->printf(", clicks=%d\n", (Fl::event_clicks() + 1));
        }
        else {
            tty->printf("\n");
        }
    }
    else {
        tty->printf("TREE CALLBACK: reason=%s, changed=%d, item=(no item -- probably multiple items were changed at once)\n",
            reason_as_name(tree->callback_reason()),
            tree->changed() ? 1 : 0);
    }
    tree->clear_changed();
}

Fl_Value_Slider* margintop_slider = (Fl_Value_Slider*)0;

static void cb_margintop_slider(Fl_Value_Slider*, void*) {
    int val = (int)margintop_slider->value();
    tree->margintop(val);
    tree->redraw();
}

Fl_Value_Slider* marginleft_slider = (Fl_Value_Slider*)0;

static void cb_marginleft_slider(Fl_Value_Slider*, void*) {
    int val = (int)marginleft_slider->value();
    tree->marginleft(val);
    tree->redraw();
}

Fl_Value_Slider* marginbottom_slider = (Fl_Value_Slider*)0;

static void cb_marginbottom_slider(Fl_Value_Slider*, void*) {
    int val = (int)marginbottom_slider->value();
    tree->marginbottom(val);
    tree->redraw();
}

Fl_Value_Slider* linespacing_slider = (Fl_Value_Slider*)0;

static void cb_linespacing_slider(Fl_Value_Slider*, void*) {
    int val = (int)linespacing_slider->value();
    tree->linespacing(val);
    tree->redraw();
}

Fl_Value_Slider* usericonmarginleft_slider = (Fl_Value_Slider*)0;

static void cb_usericonmarginleft_slider(Fl_Value_Slider*, void*) {
    int val = (int)usericonmarginleft_slider->value();
    tree->usericonmarginleft(val);
    tree->redraw();
}

Fl_Value_Slider* labelmarginleft_slider = (Fl_Value_Slider*)0;

static void cb_labelmarginleft_slider(Fl_Value_Slider*, void*) {
    int val = (int)labelmarginleft_slider->value();
    tree->labelmarginleft(val);
    tree->redraw();
}

Fl_Value_Slider* widgetmarginleft_slider = (Fl_Value_Slider*)0;

static void cb_widgetmarginleft_slider(Fl_Value_Slider*, void*) {
    int val = (int)widgetmarginleft_slider->value();
    tree->widgetmarginleft(val);
    tree->redraw();
}

Fl_Value_Slider* openchild_marginbottom_slider = (Fl_Value_Slider*)0;

static void cb_openchild_marginbottom_slider(Fl_Value_Slider*, void*) {
    int val = (int)openchild_marginbottom_slider->value();
    tree->openchild_marginbottom(val);
    tree->redraw();
}

Fl_Value_Slider* connectorwidth_slider = (Fl_Value_Slider*)0;

static void cb_connectorwidth_slider(Fl_Value_Slider*, void*) {
    tree->connectorwidth((int)connectorwidth_slider->value());
}

Fl_Choice* collapseicons_chooser = (Fl_Choice*)0;

static void cb_collapseicons_chooser(Fl_Choice*, void*) {
    static const char* L_open_xpm[] = {
  #ifdef __APPLE__
      "11 11 3 1",
      ".    c #fefefe",
      "#   c #444444",
      "@    c #000000",
      "###########",
      "#.........#",
      "#.........#",
      "#....@....#",
      "#....@....#",
      "#..@@@@@..#",
      "#....@....#",
      "#....@....#",
      "#.........#",
      "#.........#",
      "###########"
  #else
        "11 11 2 1",
        ".  c None",
        "@  c #000000",
        "...........",
        "....@......",
        "....@@.....",
        "....@@@....",
        "....@@@@...",
        "....@@@@@..",
        "....@@@@...",
        "....@@@....",
        "....@@.....",
        "....@......",
        "..........."
  #endif
    };
    static Fl_Pixmap L_openpixmap(L_open_xpm);

    static const char* L_close_xpm[] = {
  #ifdef __APPLE__
      "11 11 3 1",
      ".    c #fefefe",
      "#   c #444444",
      "@    c #000000",
      "###########",
      "#.........#",
      "#.........#",
      "#.........#",
      "#.........#",
      "#..@@@@@..#",
      "#.........#",
      "#.........#",
      "#.........#",
      "#.........#",
      "###########"
  #else
        "11 11 2 1",
        ".  c None",
        "@  c #000000",
        "...........",
        "...........",
        "...........",
        "...........",
        ".@@@@@@@@@.",
        "..@@@@@@@..",
        "...@@@@@...",
        "....@@@....",
        ".....@.....",
        "...........",
        "..........."
  #endif
    };
    static Fl_Pixmap L_closepixmap(L_close_xpm);

    switch (collapseicons_chooser->value()) {
    case 0:
        tree->showcollapse(1);
        tree->openicon(0);
        tree->closeicon(0);
        break;
    case 1:
        tree->showcollapse(1);
        tree->openicon(&L_openpixmap);
        tree->closeicon(&L_closepixmap);
        break;
    case 2:
        tree->showcollapse(0);
        break;
    }
}

Fl_Menu_Item menu_collapseicons_chooser[] = {
 {"Default", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Custom", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Off", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Choice* connectorstyle_chooser = (Fl_Choice*)0;

static void cb_connectorstyle_chooser(Fl_Choice*, void*) {
    // CHANGE COLLAPSESTYLE
    switch (connectorstyle_chooser->value()) {
    case 0: tree->connectorstyle(FL_TREE_CONNECTOR_NONE);     break;
    case 1: tree->connectorstyle(FL_TREE_CONNECTOR_DOTTED);   break;
    case 2: tree->connectorstyle(FL_TREE_CONNECTOR_SOLID);    break;
    }
}

Fl_Menu_Item menu_connectorstyle_chooser[] = {
 {"None", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Dotted", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Solid", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Choice* selectmode_chooser = (Fl_Choice*)0;

static void cb_selectmode_chooser(Fl_Choice*, void*) {
    // Set selection mode
    switch (selectmode_chooser->value()) {
    case 0:  tree->selectmode(FL_TREE_SELECT_NONE);   break;    // None
    case 1:  tree->selectmode(FL_TREE_SELECT_SINGLE); break;    // Single
    case 2:  tree->selectmode(FL_TREE_SELECT_MULTI);  break;    // Multi
    case 3:  tree->selectmode(FL_TREE_SELECT_SINGLE_DRAGGABLE);  break; // Single draggable
    default: tree->selectmode(FL_TREE_SELECT_SINGLE); break;    // Single
    }
}

Fl_Menu_Item menu_selectmode_chooser[] = {
 {"None", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Single", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Multi", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Single + drag", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Choice* reselectmode_chooser = (Fl_Choice*)0;

static void cb_reselectmode_chooser(Fl_Choice*, void*) {
    // Set reselection mode
    switch (reselectmode_chooser->value()) {
    case 0: tree->item_reselect_mode(FL_TREE_SELECTABLE_ONCE);   break;
    case 1: tree->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS); break;
    }
}

Fl_Menu_Item menu_reselectmode_chooser[] = {
 {"Selectable Once", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Selectable Always", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Choice* whenmode_chooser = (Fl_Choice*)0;

static void cb_whenmode_chooser(Fl_Choice*, void*) {
    // Set when mode
    switch (whenmode_chooser->value()) {
    case 0:  tree->when(FL_WHEN_RELEASE);   break;
    case 1:  tree->when(FL_WHEN_CHANGED);   break;
    case 2:  tree->when(FL_WHEN_NEVER);     break;
    default: tree->when(FL_WHEN_RELEASE);   break;
    }
}

Fl_Menu_Item menu_whenmode_chooser[] = {
 {"Changed", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Released", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Never", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Check_Button* usericon_radio = (Fl_Check_Button*)0;

static void cb_usericon_radio(Fl_Check_Button*, void*) {
    AssignUserIcons();
}

Fl_Check_Button* showroot_radio = (Fl_Check_Button*)0;

static void cb_showroot_radio(Fl_Check_Button*, void*) {
    int onoff = showroot_radio->value();
    tree->showroot(onoff);
}

Fl_Check_Button* visiblefocus_checkbox = (Fl_Check_Button*)0;

static void cb_visiblefocus_checkbox(Fl_Check_Button*, void*) {
    int onoff = visiblefocus_checkbox->value();
    tree->visible_focus(onoff);
}

Fl_Check_Button* labelandwidget_radio = (Fl_Check_Button*)0;

static void cb_labelandwidget_radio(Fl_Check_Button*, void*) {
    int flags = tree->item_draw_mode();
    if (labelandwidget_radio->value())
    {
        flags |= FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET;
    }
    else
    {
        flags &= ~FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET;
    }
    tree->item_draw_mode(flags);
    tree->redraw();
}

Fl_Check_Button* itemheightfromwidget_radio = (Fl_Check_Button*)0;

static void cb_itemheightfromwidget_radio(Fl_Check_Button*, void*) {
    int flags = tree->item_draw_mode();
    if (itemheightfromwidget_radio->value())
    {
        flags |= FL_TREE_ITEM_HEIGHT_FROM_WIDGET;
    }
    else
    {
        flags &= ~FL_TREE_ITEM_HEIGHT_FROM_WIDGET;
    }
    tree->item_draw_mode(flags);
    tree->redraw();
}

Fl_Check_Button* globalvisiblefocus_checkbox = (Fl_Check_Button*)0;

static void cb_globalvisiblefocus_checkbox(Fl_Check_Button*, void*) {
    int onoff = globalvisiblefocus_checkbox->value();
    Fl::visible_focus(onoff);
}

Fl_Group* showitem_box = (Fl_Group*)0;

static void cb_Show(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->next_selected_item();
    tree->show_item(item);
}

static void cb_Top(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->next_selected_item();
    tree->show_item_top(item);
}

static void cb_Mid(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->next_selected_item();
    tree->show_item_middle(item);
}

static void cb_Bot(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->next_selected_item();
    tree->show_item_bottom(item);
}

Fl_Button* openall_button = (Fl_Button*)0;

static void cb_openall_button(Fl_Button*, void*) {
    for (Fl_Tree_Item* item = tree->first();
        item;
        item = tree->next(item)) {
        if (item->has_children())
            item->open();
    }
    tree->redraw();
}

Fl_Button* loaddb_button = (Fl_Button*)0;

static void cb_loaddb_button(Fl_Button*, void*) {
    const char* filename = fl_file_chooser("Select a Preferences style Database", "Preferences(*.prefs)", 0L);
    if (filename) {
        tree->clear();
        Fl_Preferences prefs(filename, 0L, 0L, Fl_Preferences::C_LOCALE);
        tree->load(prefs);
        tree->redraw();
    }
}

Fl_Button* insertabove_button = (Fl_Button*)0;

static void cb_insertabove_button(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->first();
    while (item) {
        if (item->is_selected()) {
            tree->insert_above(item, "AaaAaa");
            tree->insert_above(item, "BbbBbb");
            tree->insert_above(item, "CccCcc");
        }
        item = item->next();
    }

    tree->redraw();
}

Fl_Button* rebuildtree_button = (Fl_Button*)0;

static void cb_rebuildtree_button(Fl_Button*, void*) {
    RebuildTree();
}

Fl_Button* showpathname_button = (Fl_Button*)0;

static void cb_showpathname_button(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->first_selected_item();
    if (!item) { fl_message("No item was selected"); return; }

    char pathname[256];
    switch (tree->item_pathname(pathname, sizeof(pathname), item)) {
    case  0: fl_message("Pathname for '%s' is: \"%s\"", (item->label() ? item->label() : "???"), pathname); break;
    case -1: fl_message("item_pathname() returned -1 (NOT FOUND)"); break;
    case -2: fl_message("item_pathname() returned -2 (STRING TOO LONG)"); break;
    }
}

Fl_Button* closeall_button = (Fl_Button*)0;

static void cb_closeall_button(Fl_Button*, void*) {
    for (Fl_Tree_Item* item = tree->first();
        item;
        item = tree->next(item)) {
        if (!item->is_root() && item->has_children())
            item->close();
    }
    tree->redraw();
}

Fl_Button* clearall_button = (Fl_Button*)0;

static void cb_clearall_button(Fl_Button*, void*) {
    tree->clear();
    tree->redraw();
}

Fl_Button* testcallbackflag_button = (Fl_Button*)0;

static void cb_testcallbackflag_button(Fl_Button*, void*) {
    Fl_Tree_Item* root = tree->root();
    tty->printf("--- Checking docallback off\n");
    if (!root) return;

    //// "OFF" TEST

    // open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->close(root, 0);       if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n close(item) triggered cb!");
    G_cb_counter = 0; tree->open(root, 0);        if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n open(item) triggered cb!");
    G_cb_counter = 0; tree->open_toggle(root, 0); if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n open_toggle(item) triggered cb!");
    G_cb_counter = 0; tree->open("ROOT", 0);      if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n open(path) triggered cb!");
    G_cb_counter = 0; tree->close("ROOT", 0);     if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n close(path) triggered cb!");
    tree->open(root, 0);     // leave root open

    // select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->select(root, 0);        if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n select(item) triggered cb!");
    G_cb_counter = 0; tree->deselect(root, 0);      if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n deselect(item) triggered cb!");
    G_cb_counter = 0; tree->select_toggle(root, 0); if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n select_toggle(item) triggered cb!");
    G_cb_counter = 0; tree->deselect("ROOT", 0);    if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n deselect(path) triggered cb!");
    G_cb_counter = 0; tree->select("ROOT", 0);      if (G_cb_counter) fl_alert("FAILED 'OFF' TEST\n select(path) triggered cb!");
    tree->deselect("ROOT"); // leave deselected

    //// "ON" TEST

    // open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->close(root, 1);       if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n close(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open(root, 1);        if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n open(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open_toggle(root, 1); if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n open_toggle(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open(root, 1);        if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n open(item)[2] cb wasn't triggered!");
    G_cb_counter = 0; tree->close(root, 1);       if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n close(item)[2] cb wasn't triggered!");
    G_cb_counter = 0; tree->open("ROOT", 1);      if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n open(path) cb wasn't triggered!");
    G_cb_counter = 0; tree->close("ROOT", 1);     if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n close(path) cb wasn't triggered!");
    tree->open(root, 0);     // leave root open

    // select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->select(root, 1);        if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n select(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->deselect(root, 1);      if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n deselect(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->select_toggle(root, 1); if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n select_toggle(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->deselect("ROOT", 1);    if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n deselect(path) cb wasn't triggered!");
    G_cb_counter = 0; tree->select("ROOT", 1);      if (!G_cb_counter) fl_alert("FAILED 'ON' TEST\n select(path) cb wasn't triggered!");
    tree->deselect("ROOT"); // leave deselected

    //// "default" TEST (should be same as 'on'

    // open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->close(root);       if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST: close(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open(root);        if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST: open(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open_toggle(root); if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST: open_toggle(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->open("ROOT");      if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST: open(path) cb wasn't triggered!");
    G_cb_counter = 0; tree->close("ROOT");     if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST: close(path) cb wasn't triggered!");
    tree->open(root, 0);     // leave root open

    // select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0; tree->select(root);        if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST\n select(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->deselect(root);      if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST\n deselect(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->select_toggle(root); if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST\n select_toggle(item) cb wasn't triggered!");
    G_cb_counter = 0; tree->deselect("ROOT");    if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST\n deselect(path) cb wasn't triggered!");
    G_cb_counter = 0; tree->select("ROOT");      if (!G_cb_counter) fl_alert("FAILED 'DEFAULT' TEST\n select(path) cb wasn't triggered!");
    tree->deselect("ROOT"); // leave deselected

    fl_alert("TEST COMPLETED\n If you didn't see any error dialogs, test PASSED.");
}

Fl_Button* testrootshowself_button = (Fl_Button*)0;

static void cb_testrootshowself_button(Fl_Button*, void*) {
    Fl_Tree_Item* root = tree->root();
    tty->printf("--- Show Tree\n");
    if (root) root->show_self();
}

Fl_Button* add20k_button = (Fl_Button*)0;

static void cb_add20k_button(Fl_Button*, void*) {
    static int item_id = 501;
    Fl_Tree_Item* item = tree->first();
    while (item) {
        if (item->is_selected()) {
            Fl_Tree_Item* parent = item->parent();
            if (parent == 0) parent = tree->root();
            char s[80];
            for (int i = 0; i < 20000; i++) {
                snprintf(s, 80, "Item #%d", item_id + i);
                tree->add(parent, s);
            }
            item_id += 20000;
            break;
        }
        item = item->next();
    }
    tree->redraw();
}

Fl_Choice* selected_labelfont_choice = (Fl_Choice*)0;

static void cb_selected_labelfont_choice(Fl_Choice*, void*) {
    // Find first item in tree
    Fl_Tree_Item* item = tree->first();
    if (!item) return;

    // Get first item's font.
    Fl_Font val = (Fl_Font)selected_labelfont_choice->value(); // Get font value

    // Do selected items
    int count = 0;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->labelfont(val);
            count++;
        }
    }

    // No items selected? Do all..
    if (!count) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->labelfont(val);
        }
    }

    tree->redraw();
}

Fl_Menu_Item menu_selected_labelfont_choice[] = {
 {"Helvetica", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Symbol", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Zapf Dingbats", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Value_Slider* selected_labelsize_slider = (Fl_Value_Slider*)0;

static void cb_selected_labelsize_slider(Fl_Value_Slider*, void*) {
    int size = (int)selected_labelsize_slider->value();

    // DO SELECTED ITEMS
    int count = 0;
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->labelsize(size);
            count++;
        }
    }

    // NO ITEMS SELECTED? DO ALL
    if (!count) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->labelsize(size);
        }
    }

    tree->redraw();
}

Fl_Button* all_labelfgcolor_button = (Fl_Button*)0;

static void cb_all_labelfgcolor_button(Fl_Button*, void*) {
    // Get first item's color
    Fl_Color val = GetSelectedItemFGColor();                // Get color of first selected item
    if (EditColor(val) == 0) return;                      // Let user edit color. (return if they hit 'Cancel')
    all_labelfgcolor_button->color(val);                    // update modified color to button

    // Do selected items
    int count = 0;
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->labelfgcolor(val);
            count++;
        }
    }

    // No items selected? Do all..
    if (!count) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->labelfgcolor(val);
        }
    }

    tree->redraw();
}

Fl_Button* all_labelbgcolor_button = (Fl_Button*)0;

static void cb_all_labelbgcolor_button(Fl_Button*, void*) {
    // Get first item's color
    Fl_Color val = GetSelectedItemBGColor();                // Get color of first selected item
    if (EditColor(val) == 0) return;                      // Let user edit color. (return if they hit 'Cancel')
    all_labelbgcolor_button->color(val);                    // update modified color to button

    // Do selected items
    int count = 0;
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->labelbgcolor(val);
            count++;
        }
    }

    // No items selected? Do all..
    if (!count) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->labelbgcolor(val);
        }
    }

    tree->redraw();
}

Fl_Light_Button* deactivate_items_toggle = (Fl_Light_Button*)0;

static void cb_deactivate_items_toggle(Fl_Light_Button*, void*) {
    int onoff = deactivate_items_toggle->value() ? 0 : 1;

    Fl_Tree_Item* item;
    int count = 0;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->activate(onoff);
            ++count;
        }
    }

    if (count == 0) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->activate(onoff);
        }
    }

    tree->redraw();
}

Fl_Light_Button* deactivate_tree_toggle = (Fl_Light_Button*)0;

static void cb_deactivate_tree_toggle(Fl_Light_Button*, void*) {
    if (deactivate_tree_toggle->value())
        tree->deactivate();
    else
        tree->activate();
}

Fl_Light_Button* bold_toggle = (Fl_Light_Button*)0;

static void cb_bold_toggle(Fl_Light_Button*, void*) {
    int face = bold_toggle->value() ? FL_HELVETICA_BOLD : FL_HELVETICA;

    // DO SELECTED ITEMS
    int count = 0;
    Fl_Tree_Item* item;
    for (item = tree->first(); item; item = tree->next(item)) {
        if (item->is_selected()) {
            item->labelfont(face);
            count++;
        }
    }

    // NO ITEMS SELECTED? DO ALL
    if (!count) {
        for (item = tree->first(); item; item = tree->next(item)) {
            item->labelfont(face);
        }
    }

    tree->redraw();
}

Fl_Button* showselected_button = (Fl_Button*)0;

static void cb_showselected_button(Fl_Button*, void*) {
    tty->printf("--- SELECTED ITEMS\n");
    for (Fl_Tree_Item* item = tree->first_selected_item();
        item;
        item = tree->next_selected_item(item)) {
        tty->printf("\t%s\n", item->label() ? item->label() : "???");
    }
}

Fl_Button* clearselected_button = (Fl_Button*)0;

static void cb_clearselected_button(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->first();
    while (item) {
        if (item->is_selected()) {
            if (tree->remove(item) == -1) break;
            item = tree->first();
        }
        else {
            item = item->next();
        }
    }

    tree->redraw();
}

Fl_Button* swapselected_button = (Fl_Button*)0;

static void cb_swapselected_button(Fl_Button*, void*) {
    Fl_Tree_Item* item = tree->first();
    Fl_Tree_Item* a = 0, * b = 0;
    while (item) {
        if (item->is_selected()) {
            if (!a) a = item;
            else if (!b) b = item;
            else {
                fl_alert("Too many items selected. (must select only two)");
                return;
            }
        }
        item = item->next();
    }
    if (!a || !b) {
        fl_alert("Too few items selected. (you must select two)");
        return;
    }
    Fl_Tree_Item* pa = a->parent();
    Fl_Tree_Item* pb = b->parent();
    if (pa != pb) {
        fl_alert("The two selected items must be siblings");
        return;
    }
    pa->swap_children(a, b);
    tree->redraw();
}

Fl_Button* selectall_button = (Fl_Button*)0;

static void cb_selectall_button(Fl_Button*, void*) {
    tree->select_all(0);
    tree->redraw();
}

Fl_Button* deselectall_button = (Fl_Button*)0;

static void cb_deselectall_button(Fl_Button*, void*) {
    tree->deselect_all(0);
    tree->redraw();
}

Fl_Button* nextselected_button = (Fl_Button*)0;

static void cb_nextselected_button(Fl_Button*, void*) {
    tty->printf("--- TEST next_selected():\n");
    tty->printf("    // Walk down the tree (forwards)\n");
    for (Fl_Tree_Item* i = tree->first_selected_item(); i; i = tree->next_selected_item(i, FL_Down)) {
        tty->printf("    Selected item: %s\n", i->label() ? i->label() : "<nolabel>");
    }

    tty->printf("    // Walk up the tree (backwards)\n");
    for (Fl_Tree_Item* i = tree->last_selected_item(); i; i = tree->next_selected_item(i, FL_Up)) {
        tty->printf("    Selected item: %s\n", i->label() ? i->label() : "<nolabel>");
    }
}

Fl_Light_Button* bbbselect_toggle = (Fl_Light_Button*)0;

static void cb_bbbselect_toggle(Fl_Light_Button*, void*) {
    // Toggle select of just the Bbb item (not children)
    Fl_Tree_Item* bbb = tree->find_item("/Bbb");
    if (!bbb) {
        fl_alert("FAIL: Couldn't find item '/Bbb'???");
        return;
    }
    int onoff = bbbselect_toggle->value();
    if (onoff) tree->select(bbb);         // select /Bbb
    else         tree->deselect(bbb);       // deselect /Bbb;
}

Fl_Light_Button* bbbselect2_toggle = (Fl_Light_Button*)0;

static void cb_bbbselect2_toggle(Fl_Light_Button*, void*) {
    // Toggle select of just the Bbb item and its immediate children
    Fl_Tree_Item* bbb = tree->find_item("/Bbb");
    if (!bbb) {
        fl_alert("FAIL: Couldn't find item '/Bbb'???");
        return;
    }
    int onoff = bbbselect2_toggle->value();
    if (onoff) tree->select_all(bbb);             // select /Bbb and its children
    else         tree->deselect_all(bbb);   // deselect /Bbb and its children;
}

Fl_Light_Button* bbbchild02select_toggle = (Fl_Light_Button*)0;

static void cb_bbbchild02select_toggle(Fl_Light_Button*, void*) {
    // Toggle select of just the /Bbb/child-02 item
    const char* pathname = "/Bbb/child-02";
    int onoff = bbbchild02select_toggle->value();
    int err = 0;
    if (onoff) err = tree->select(pathname);
    else         err = tree->deselect(pathname);

    if (err == -1) {
        fl_alert("FAIL: Couldn't find item '%s'", pathname);
        return;
    }
}

Fl_Light_Button* rootselect_toggle = (Fl_Light_Button*)0;

static void cb_rootselect_toggle(Fl_Light_Button*, void*) {
    // Toggle select of ROOT item and its children
    Fl_Tree_Item* item = tree->find_item("/ROOT");
    if (!item) {
        fl_alert("FAIL: Couldn't find item '/ROOT'???");
        return;
    }
    int onoff = rootselect_toggle->value();
    if (onoff) tree->select(item);        // select /ROOT and its children
    else         tree->deselect(item);      // deselect /ROOT and its children;
}

Fl_Light_Button* rootselect2_toggle = (Fl_Light_Button*)0;

static void cb_rootselect2_toggle(Fl_Light_Button*, void*) {
    // Toggle select of ROOT item and its children
    Fl_Tree_Item* item = tree->find_item("/ROOT");
    if (!item) {
        fl_alert("FAIL: Couldn't find item '/ROOT'???");
        return;
    }
    int onoff = rootselect2_toggle->value();
    if (onoff) tree->select_all(item);    // select /ROOT and its children
    else         tree->deselect_all(item);  // deselect /ROOT and its children;
}

Fl_Choice* labelfont_choice = (Fl_Choice*)0;

static void cb_labelfont_choice(Fl_Choice*, void*) {
    Fl_Font val = (Fl_Font)labelfont_choice->value();
    tree->labelfont(val);
    window->redraw();
}

Fl_Menu_Item menu_labelfont_choice[] = {
 {"Helvetica", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Symbol", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Zapf Dingbats", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Value_Slider* labelsize_slider = (Fl_Value_Slider*)0;

static void cb_labelsize_slider(Fl_Value_Slider*, void*) {
    tree->labelsize((int)labelsize_slider->value());
    window->redraw();
}

Fl_Choice* item_labelfont_choice = (Fl_Choice*)0;

static void cb_item_labelfont_choice(Fl_Choice*, void*) {
    Fl_Font val = (Fl_Font)item_labelfont_choice->value();
    tree->item_labelfont(val);
    tree->redraw();
}

Fl_Menu_Item menu_item_labelfont_choice[] = {
 {"Helvetica", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Helvetica Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Courier Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Times Bold Italic", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Symbol", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Screen bold", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {"Zapf Dingbats", 0,  0, 0, 0, (uchar)FL_NORMAL_LABEL, 0, 12, 0},
 {0,0,0,0,0,0,0,0,0}
};

Fl_Value_Slider* item_labelsize_slider = (Fl_Value_Slider*)0;

static void cb_item_labelsize_slider(Fl_Value_Slider*, void*) {
    tree->item_labelsize((int)item_labelsize_slider->value());
    tree->redraw();
}

Fl_Button* labelcolor_button = (Fl_Button*)0;

static void cb_labelcolor_button(Fl_Button*, void*) {
    Fl_Color val = tree->labelcolor();
    if (EditColor(val) == 0) return;                      // Let user edit color. (return if they hit 'Cancel')
    labelcolor_button->color(val);                          // update modified color to button
    tree->labelcolor(val);
    window->redraw();   // affects window (tree's label is outside tree's area);
}

Fl_Button* color_button = (Fl_Button*)0;

static void cb_color_button(Fl_Button*, void*) {
    Fl_Color val = tree->color();
    if (EditColor(val) == 0) return;                      // Let user edit color. (return if they hit 'Cancel')
    color_button->color(val);       // update modified color to button
    tree->color(val);
    UpdateColorChips();
    tree->redraw();
}

Fl_Button* selection_color_button = (Fl_Button*)0;

static void cb_selection_color_button(Fl_Button*, void*) {
    Fl_Color val = tree->selection_color();
    if (EditColor(val) == 0) return;              // Let user edit color. (return if they hit 'Cancel')
    selection_color_button->color(val);             // update modified color to button
    tree->selection_color(val);
    tree->redraw();
}

Fl_Button* item_labelfgcolor_button = (Fl_Button*)0;

static void cb_item_labelfgcolor_button(Fl_Button*, void*) {
    Fl_Color val = tree->item_labelfgcolor();
    if (EditColor(val) == 0) return;              // Let user edit color. (return if they hit 'Cancel')
    tree->item_labelfgcolor(val);                   // apply modified color to tree
    item_labelfgcolor_button->color(val);           // update modified color to button
    tree->redraw();
}

Fl_Button* item_labelbgcolor_button = (Fl_Button*)0;

static void cb_item_labelbgcolor_button(Fl_Button*, void*) {
    Fl_Color val = tree->item_labelbgcolor();
    if (EditColor(val) == 0) return;              // Let user edit color. (return if they hit 'Cancel')
    tree->item_labelbgcolor(val);                   // apply modified color to tree
    item_labelbgcolor_button->color(val);           // update modified color to button
    tree->redraw();
}

Fl_Button* x_item_labelbgcolor_button = (Fl_Button*)0;

static void cb_x_item_labelbgcolor_button(Fl_Button*, void*) {
    tree->item_labelbgcolor(0xffffffff);
    UpdateColorChips();
    tree->redraw();
}

Fl_Value_Slider* tree_scrollbar_size_slider = (Fl_Value_Slider*)0;

static void cb_tree_scrollbar_size_slider(Fl_Value_Slider*, void*) {
    tree->scrollbar_size((int)tree_scrollbar_size_slider->value());
    tree->redraw();
}

Fl_Value_Slider* scrollbar_size_slider = (Fl_Value_Slider*)0;

static void cb_scrollbar_size_slider(Fl_Value_Slider*, void*) {
    Fl::scrollbar_size((int)scrollbar_size_slider->value());
    tree->redraw();
}

Fl_Button* testsuggs_button = (Fl_Button*)0;

static void cb_testsuggs_button(Fl_Button*, void*) {
    const char* helpmsg =
        "CHILD WIDGET SIZING TESTS\n"
        "=========================\n"
        "   1) Start program\n"
        "   2) Click the 'ccc button' and D1/D2 buttons.\n"
        "      Their sizes should not change.\n"
        "   3) Click the 'Item h() from widget' checkbox.\n"
        "   4) Click the 'ccc button' and D1/D2 buttons.\n"
        "      Their sizes should change, getting larger vertically.\n"
        "      This validates that widget's size can affect the tree.\n"
        "   5) Disable the checkbox, widgets should resize back to the\n"
        "      size of the other items.\n"
        "   6) Hit ^A to select all items\n"
        "   7) Under 'Selected Items', drag the 'Label Size' slider around.\n"
        "      All the item's height should change, as well as child widgets.\n"
        "   8) Put Label Size' slider back to normal\n"
        "\n"
        "CHILD WIDGET + LABEL ITEM DRAWING TESTS\n"
        "=======================================\n"
        "   1) Start program\n"
        "   2) Click 'Show label + widget'.\n"
        "      The widgets should all show item labels to their left.\n"
        "   3) Disable same, item labels should disappear,\n"
        "      showing the widgets in their place.\n"
        "\n"
        "COLORS\n"
        "======\n"
        "   1) Start program\n"
        "   2) Change 'Tree Fonts+Colors' -> color()\n"
        "   3) Entire tree's background color will change, including items.\n"
        "   4) Change the 'Tree Fonts + Colors -> item_labelbgcolor()'\n"
        "   6) Click the '111' item to select it.\n"
        "   7) Click 'Test Operations -> Insert Above'\n"
        "      New items should appear above the selected item using the new color.\n"
        "      This color will be different from the background color.\n"
        "   8) Change the 'Tree Fonts+Colors' -> color()\n"
        "      The entire tree's bg should change, except the new items.\n"
        "   9) Click the Tree Fonts+Colors -> item_labelbgcolor() 'X' button.\n"
        "      This resets item_labelbgcolor() to the default 'transparent' color (0xffffffff)\n"
        "  10) Again, click the 'Insert Above' button.\n"
        "      New items will be created in the background color, and changing the color()\n"
        "      should affect the new items too.\n"
        "\n"
        "SCROLLING\n"
        "=========\n"
        "   1) Open '500 items' and 'Long Line' so that both scrollbars appear:\n"
        "        * The 'focus box' for the selected item should not be clipped\n"
        "          horizontally by the vertical scrollbar.\n"
        "        * Resizing the window horizontally should resize the focus box\n"
        "        * Scrolling vertically/horizontally should show reveal all\n"
        "          edges of the tree. One *exception* is the widget label\n"
        "          to the right of the 'ccc button'; labels aren't part\n"
        "          of the widget, and therefore don't affect scroll tabs\n"
        "   2) Scroll vertical scroller to the middle of the tree\n"
        "   3) Left click and drag up/down to extend the selection:\n"
        "        * Selection should autoscroll if you drag off the top/bottom\n"
        "        * Selection should work *even* if you drag horizontally\n"
        "          off the window edge; moving up/down outside the window\n"
        "          should continue to autoscroll\n"
        "   4) Click either of the the scrollbar tabs and drag:\n"
        "        * Even if you drag off the scrollbar, the scrollbar\n"
        "          tab should continue to move\n"
        "        * Should continue to work if you drag off the window edge\n"
        "          horizontally drag off the window.\n"
        "   5) Click 'Bbb' and hit 'Add 20,000', then position the\n"
        "      'ccc button' so it's partially obscured by a scrollbar tab:\n"
        "        * Clicking the obscured button should work\n"
        "        * Clicking on the tab over the button should not 'click through'\n"
        "          to the button.\n"
        "";

    static Fl_Double_Window* helpwin = 0;
    static Fl_Text_Display* helpdisp = 0;
    static Fl_Text_Buffer* helpbuff = 0;
    if (!helpwin) {
        Fl_Group::current(0);  // ensure we don't become child of other win
        helpwin = new Fl_Double_Window(600, 600, "Test Suggestions");
        helpdisp = new Fl_Text_Display(0, 0, helpwin->w(), helpwin->h());
        helpbuff = new Fl_Text_Buffer();
        helpdisp->buffer(helpbuff);
        helpdisp->textfont(FL_COURIER);
        helpdisp->textsize(12);
        helpbuff->text(helpmsg);
        helpwin->end();
    }
    helpwin->resizable(helpdisp);
    helpwin->show();
}

Fl_Box* resizer_box = (Fl_Box*)0;

Fl_Terminal* tty = (Fl_Terminal*)0;

#ifdef _DEBUG
int main(int argc, char* argv[])
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
#endif
{
#ifndef _DEBUG
    int rc;
    int i;
    int argc = 0;
    char** argv = NULL;

    /* Get the command line arguments as Windows Wide Character strings */
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

    /* Allocate an array of 'argc + 1' string pointers */
    argv = (char**)malloc((argc + 1) * sizeof(char*));

    /* Convert the command line arguments to UTF-8 */
    for (i = 0; i < argc; i++) {
        /* find the required size of the buffer */
        int u8size = WideCharToMultiByte(CP_UTF8,     /* CodePage */
            0,           /* dwFlags */
            wideArgv[i], /* lpWideCharStr */
            -1,          /* cchWideChar */
            NULL,        /* lpMultiByteStr */
            0,           /* cbMultiByte */
            NULL,        /* lpDefaultChar */
            NULL);       /* lpUsedDefaultChar */
        if (u8size > 0) {
            char* strbuf = (char*)malloc(u8size);
            int ret = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                0,           /* dwFlags */
                wideArgv[i], /* lpWideCharStr */
                -1,          /* cchWideChar */
                strbuf,      /* lpMultiByteStr */
                u8size,      /* cbMultiByte */
                NULL,        /* lpDefaultChar */
                NULL);       /* lpUsedDefaultChar */
            if (ret) {
                argv[i] = strbuf;
            }
            else {
                argv[i] = _strdup("");
                free(strbuf);
            }
        }
        else {
            argv[i] = _strdup("");
        }
    }
    argv[argc] = NULL; /* required by C standard at end of list */

    /* Free the wide character string array */
    LocalFree(wideArgv);
#endif

    //Fl_Window* window = new Fl_Window(340, 180);
    //Fl_Box* box = new Fl_Box(20, 40, 300, 100, "Hello, World!");
    //box->box(FL_UP_BOX);
    //box->labelfont(FL_BOLD + FL_ITALIC);
    //box->labelsize(36);
    //box->labeltype(FL_SHADOW_LABEL);
    //window->end();
    //window->show(argc, argv);
    //Fl::run();

    { window = new Fl_Double_Window(1045, 730, "tree");
    { tree = new Fl_Tree(15, 22, 320, 539, "Tree");
    tree->tooltip("Test tree");
    tree->box(FL_DOWN_BOX);
    tree->color((Fl_Color)55);
    tree->selection_color(FL_SELECTION_COLOR);
    tree->labeltype(FL_NORMAL_LABEL);
    tree->labelfont(0);
    tree->labelsize(14);
    tree->labelcolor(FL_FOREGROUND_COLOR);
    tree->callback((Fl_Callback*)cb_tree, (void*)(1234));
    tree->align(Fl_Align(FL_ALIGN_TOP));
    tree->when(FL_WHEN_RELEASE);
    tree->end();
} // Fl_Tree* tree
    { Fl_Group* o = new Fl_Group(350, 5, 681, 556);
    { Fl_Group* o = new Fl_Group(350, 23, 330, 389, "Tree Globals");
    o->tooltip("These controls only affect the selected items. If no items are selected, all "
        "existing items in tree are modified.");
    o->box(FL_GTK_DOWN_BOX);
    o->color(FL_DARK1);
    o->labelsize(12);
    { Fl_Value_Slider* o = margintop_slider = new Fl_Value_Slider(505, 36, 155, 16, "margintop()");
    margintop_slider->tooltip("Changes the top margin for the tree widget");
    margintop_slider->type(1);
    margintop_slider->color((Fl_Color)46);
    margintop_slider->selection_color((Fl_Color)1);
    margintop_slider->labelsize(10);
    margintop_slider->textsize(9);
    margintop_slider->callback((Fl_Callback*)cb_margintop_slider, (void*)(tree));
    margintop_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->margintop());
    o->range(0.0, 100.0);
    o->step(1.0);
    } // Fl_Value_Slider* margintop_slider
    { Fl_Value_Slider* o = marginleft_slider = new Fl_Value_Slider(505, 56, 155, 16, "marginleft()");
    marginleft_slider->tooltip("Changes the left margin for the tree widget");
    marginleft_slider->type(1);
    marginleft_slider->color((Fl_Color)46);
    marginleft_slider->selection_color((Fl_Color)1);
    marginleft_slider->labelsize(10);
    marginleft_slider->textsize(9);
    marginleft_slider->callback((Fl_Callback*)cb_marginleft_slider, (void*)(tree));
    marginleft_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->marginleft());
    o->range(0.0, 200.0);
    o->step(1.0);
    } // Fl_Value_Slider* marginleft_slider
    { Fl_Value_Slider* o = marginbottom_slider = new Fl_Value_Slider(505, 76, 155, 16, "marginbottom()");
    marginbottom_slider->tooltip("Changes the bottom margin for the tree\nSets how far beyond bottom of tree yo"
        "u can scroll");
    marginbottom_slider->type(1);
    marginbottom_slider->color((Fl_Color)46);
    marginbottom_slider->selection_color((Fl_Color)1);
    marginbottom_slider->labelsize(10);
    marginbottom_slider->textsize(9);
    marginbottom_slider->callback((Fl_Callback*)cb_marginbottom_slider, (void*)(tree));
    marginbottom_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(GetTreeMarginBottom()); // handle ABI feature
    o->range(0.0, 275.0);
    o->step(1.0);
    o->do_callback();
    } // Fl_Value_Slider* marginbottom_slider
    { Fl_Value_Slider* o = linespacing_slider = new Fl_Value_Slider(505, 96, 155, 16, "linespacing()");
    linespacing_slider->tooltip("Changes the spacing between items in the tree");
    linespacing_slider->type(1);
    linespacing_slider->color((Fl_Color)46);
    linespacing_slider->selection_color((Fl_Color)1);
    linespacing_slider->labelsize(10);
    linespacing_slider->textsize(9);
    linespacing_slider->callback((Fl_Callback*)cb_linespacing_slider, (void*)(tree));
    linespacing_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->linespacing());
    o->range(0.0, 100.0);
    o->step(1.0);
    } // Fl_Value_Slider* linespacing_slider
    { Fl_Value_Slider* o = usericonmarginleft_slider = new Fl_Value_Slider(505, 116, 155, 16, "usericonmarginleft()");
    usericonmarginleft_slider->tooltip("Changes the left margin for the user icons (if any)");
    usericonmarginleft_slider->type(1);
    usericonmarginleft_slider->color((Fl_Color)46);
    usericonmarginleft_slider->selection_color((Fl_Color)1);
    usericonmarginleft_slider->labelsize(10);
    usericonmarginleft_slider->textsize(9);
    usericonmarginleft_slider->callback((Fl_Callback*)cb_usericonmarginleft_slider, (void*)(tree));
    usericonmarginleft_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->usericonmarginleft());
    o->range(0.0, 100.0);
    o->step(1.0);
    } // Fl_Value_Slider* usericonmarginleft_slider
    { Fl_Value_Slider* o = labelmarginleft_slider = new Fl_Value_Slider(505, 136, 155, 16, "labelmarginleft()");
    labelmarginleft_slider->tooltip("Changes the left margin for the item label");
    labelmarginleft_slider->type(1);
    labelmarginleft_slider->color((Fl_Color)46);
    labelmarginleft_slider->selection_color((Fl_Color)1);
    labelmarginleft_slider->labelsize(10);
    labelmarginleft_slider->textsize(9);
    labelmarginleft_slider->callback((Fl_Callback*)cb_labelmarginleft_slider, (void*)(tree));
    labelmarginleft_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->labelmarginleft());
    o->range(0.0, 100.0);
    o->step(1.0);
    } // Fl_Value_Slider* labelmarginleft_slider
    { Fl_Value_Slider* o = widgetmarginleft_slider = new Fl_Value_Slider(505, 156, 155, 16, "widgetmarginleft()");
    widgetmarginleft_slider->tooltip("Changes the margin to the left of child FLTK widget()\n\"Show label + widget"
        "\" must be \'on\' for this to take effect, i.e.\nitem_draw_mode(FL_TREE_ITEM_D"
        "RAW_LABEL_AND_WIDGET)");
    widgetmarginleft_slider->type(1);
    widgetmarginleft_slider->color((Fl_Color)46);
    widgetmarginleft_slider->selection_color((Fl_Color)1);
    widgetmarginleft_slider->labelsize(10);
    widgetmarginleft_slider->textsize(9);
    widgetmarginleft_slider->callback((Fl_Callback*)cb_widgetmarginleft_slider, (void*)(tree));
    widgetmarginleft_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(GetTreeWidgetMarginLeft());  // handle ABI feature
    o->range(0.0, 100.0);
    o->step(1.0);
    o->do_callback();
    } // Fl_Value_Slider* widgetmarginleft_slider
    { Fl_Value_Slider* o = openchild_marginbottom_slider = new Fl_Value_Slider(505, 176, 155, 16, "openchild_marginbottom()");
    openchild_marginbottom_slider->tooltip("Changes the vertical space below an open child tree");
    openchild_marginbottom_slider->type(1);
    openchild_marginbottom_slider->color((Fl_Color)46);
    openchild_marginbottom_slider->selection_color((Fl_Color)1);
    openchild_marginbottom_slider->labelsize(10);
    openchild_marginbottom_slider->textsize(9);
    openchild_marginbottom_slider->callback((Fl_Callback*)cb_openchild_marginbottom_slider, (void*)(tree));
    openchild_marginbottom_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->openchild_marginbottom());
    o->range(0.0, 100.0);
    o->step(1.0);
    } // Fl_Value_Slider* openchild_marginbottom_slider
    { Fl_Value_Slider* o = connectorwidth_slider = new Fl_Value_Slider(505, 195, 155, 16, "connectorwidth()");
    connectorwidth_slider->tooltip("Tests Fl_Tree::connectorwidth()");
    connectorwidth_slider->type(1);
    connectorwidth_slider->color((Fl_Color)46);
    connectorwidth_slider->selection_color((Fl_Color)1);
    connectorwidth_slider->labelsize(11);
    connectorwidth_slider->textsize(9);
    connectorwidth_slider->callback((Fl_Callback*)cb_connectorwidth_slider, (void*)(tree));
    connectorwidth_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->connectorwidth());
    o->range(1.0, 100.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* connectorwidth_slider
    { collapseicons_chooser = new Fl_Choice(520, 221, 140, 21, "Collapse icons");
    collapseicons_chooser->tooltip("Tests Fl_Tree::openicon(), Fl_Tree::closeicon() and Fl_Tree::showcollapse().");
    collapseicons_chooser->down_box(FL_BORDER_BOX);
    collapseicons_chooser->labelsize(12);
    collapseicons_chooser->textsize(11);
    collapseicons_chooser->callback((Fl_Callback*)cb_collapseicons_chooser);
    collapseicons_chooser->menu(menu_collapseicons_chooser);
    } // Fl_Choice* collapseicons_chooser
    { connectorstyle_chooser = new Fl_Choice(520, 245, 140, 21, "Line style");
    connectorstyle_chooser->tooltip("Tests Fl_Tree::connectorstyle() bit flags");
    connectorstyle_chooser->down_box(FL_BORDER_BOX);
    connectorstyle_chooser->labelsize(12);
    connectorstyle_chooser->textsize(11);
    connectorstyle_chooser->callback((Fl_Callback*)cb_connectorstyle_chooser);
    connectorstyle_chooser->menu(menu_connectorstyle_chooser);
          switch (tree->connectorstyle()) { case FL_TREE_CONNECTOR_NONE: connectorstyle_chooser->value(0); break; case FL_TREE_CONNECTOR_DOTTED: connectorstyle_chooser->value(1); break; case FL_TREE_CONNECTOR_SOLID: connectorstyle_chooser->value(2); break; }
    } // Fl_Choice* connectorstyle_chooser
    { selectmode_chooser = new Fl_Choice(520, 269, 140, 21, "Selection Mode");
    selectmode_chooser->tooltip("Tests Fl_Tree::selectmode()\nSets how Fl_Tree handles mouse selection of tree"
        " items.\n    NONE        -- Not selectable by keyboard/mouse\n    SINGLE      "
        "-- Only one item at a time selectable by keyboard/mouse\n    MULTI       -- Mu"
        "ltiple items selectable\n    SINGLE+DRAG -- Lets user drag items to different "
        "position in tree");
    selectmode_chooser->down_box(FL_BORDER_BOX);
    selectmode_chooser->labelsize(12);
    selectmode_chooser->textsize(11);
    selectmode_chooser->callback((Fl_Callback*)cb_selectmode_chooser);
    selectmode_chooser->menu(menu_selectmode_chooser);
    selectmode_chooser->value(2);
    cb_selectmode_chooser(selectmode_chooser, (void*)0);
    } // Fl_Choice* selectmode_chooser
    { reselectmode_chooser = new Fl_Choice(520, 293, 140, 21, "Item Reselect Mode");
    reselectmode_chooser->tooltip("Tests Fl_Tree::item_reselect_mode().\nEnables \'reselect\' events.\nThese hap"
        "pen when someone selects an item already selected\n(mouse drags or multi-click"
        "s)");
    reselectmode_chooser->down_box(FL_BORDER_BOX);
    reselectmode_chooser->labelsize(12);
    reselectmode_chooser->textsize(11);
    reselectmode_chooser->callback((Fl_Callback*)cb_reselectmode_chooser);
    reselectmode_chooser->menu(menu_reselectmode_chooser);
    reselectmode_chooser->value(1);
    reselectmode_chooser->do_callback();
    } // Fl_Choice* reselectmode_chooser
    { whenmode_chooser = new Fl_Choice(520, 319, 140, 21, "When");
    whenmode_chooser->tooltip("Sets when() the tree\'s callback is invoked");
    whenmode_chooser->down_box(FL_BORDER_BOX);
    whenmode_chooser->labelsize(12);
    whenmode_chooser->textsize(11);
    whenmode_chooser->callback((Fl_Callback*)cb_whenmode_chooser);
    whenmode_chooser->menu(menu_whenmode_chooser);
    whenmode_chooser->value(1);
    cb_whenmode_chooser(whenmode_chooser, (void*)0);
    } // Fl_Choice* whenmode_chooser
    { usericon_radio = new Fl_Check_Button(485, 351, 20, 16, "Enable user icons?");
    usericon_radio->tooltip("Tests Fl_Tree_Item::usericon()");
    usericon_radio->down_box(FL_DOWN_BOX);
    usericon_radio->labelsize(11);
    usericon_radio->callback((Fl_Callback*)cb_usericon_radio, (void*)(tree));
    usericon_radio->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    usericon_radio->value(1);
    } // Fl_Check_Button* usericon_radio
    { showroot_radio = new Fl_Check_Button(485, 368, 20, 16, "Show root?");
    showroot_radio->tooltip("Tests tree->showroot();");
    showroot_radio->down_box(FL_DOWN_BOX);
    showroot_radio->labelsize(11);
    showroot_radio->callback((Fl_Callback*)cb_showroot_radio, (void*)(tree));
    showroot_radio->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    int onoff = tree->showroot(); showroot_radio->value(onoff);
    } // Fl_Check_Button* showroot_radio
    { visiblefocus_checkbox = new Fl_Check_Button(485, 385, 20, 16, "Visible focus?");
    visiblefocus_checkbox->tooltip("Toggles the tree\'s visible_focus() box");
    visiblefocus_checkbox->down_box(FL_DOWN_BOX);
    visiblefocus_checkbox->labelsize(11);
    visiblefocus_checkbox->callback((Fl_Callback*)cb_visiblefocus_checkbox, (void*)(tree));
    visiblefocus_checkbox->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    int onoff = tree->visible_focus(); visiblefocus_checkbox->value(onoff);
    } // Fl_Check_Button* visiblefocus_checkbox
    { labelandwidget_radio = new Fl_Check_Button(645, 351, 20, 16, "Show label + widget");
    labelandwidget_radio->tooltip("Tests Fl_Tree::item_draw_mode(FL_TREE_ITEM_DRAW_LABEL_AND_WIDGET)\nEnables bo"
        "th label and widget() for display.\nWhen enabled, widget should appear to the "
        "right of the item\'s label.\nBy default, the widget() is shown in place of the"
        " item\'s label.");
    labelandwidget_radio->down_box(FL_DOWN_BOX);
    labelandwidget_radio->labelsize(11);
    labelandwidget_radio->callback((Fl_Callback*)cb_labelandwidget_radio);
    labelandwidget_radio->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    labelandwidget_radio->value(0);
    labelandwidget_radio->do_callback();
    } // Fl_Check_Button* labelandwidget_radio
    { itemheightfromwidget_radio = new Fl_Check_Button(645, 368, 20, 16, "Item h() from widget");
    itemheightfromwidget_radio->tooltip("Tests Fl_Tree::item_draw_mode(FL_TREE_ITEM_HEIGHT_FROM_WIDGET)\nIf enabled, i"
        "tem\'s height will track the widget()\'s height.\nWhen enabled, click \'ccc bu"
        "tton\' or \'D1/D2\' buttons to test.");
    itemheightfromwidget_radio->down_box(FL_DOWN_BOX);
    itemheightfromwidget_radio->labelsize(11);
    itemheightfromwidget_radio->callback((Fl_Callback*)cb_itemheightfromwidget_radio);
    itemheightfromwidget_radio->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    itemheightfromwidget_radio->value(0);
    itemheightfromwidget_radio->do_callback();
    } // Fl_Check_Button* itemheightfromwidget_radio
    { globalvisiblefocus_checkbox = new Fl_Check_Button(645, 385, 20, 16, "Global visible focus?");
    globalvisiblefocus_checkbox->tooltip("Toggles the global Fl::visible_focus()");
    globalvisiblefocus_checkbox->down_box(FL_DOWN_BOX);
    globalvisiblefocus_checkbox->labelsize(11);
    globalvisiblefocus_checkbox->callback((Fl_Callback*)cb_globalvisiblefocus_checkbox, (void*)(tree));
    globalvisiblefocus_checkbox->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    int onoff = Fl::visible_focus(); globalvisiblefocus_checkbox->value(onoff);
    } // Fl_Check_Button* globalvisiblefocus_checkbox
    o->end();
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(350, 435, 330, 125, "Test Operations");
    o->tooltip("These controls only affect the defaults for new items that are created. These"
        " test the Fl_Tree_Prefs methods.");
    o->box(FL_GTK_DOWN_BOX);
    o->color(FL_DARK1);
    o->labelsize(12);
    { showitem_box = new Fl_Group(370, 460, 70, 82, "show_item()\n");
    showitem_box->box(FL_GTK_DOWN_BOX);
    showitem_box->labelsize(11);
    { Fl_Button* o = new Fl_Button(385, 469, 40, 17, "Show");
    o->tooltip("Tests show_item() with no position specified.\nMakes the selected item visibl"
        "e IF it is off-screen.\nNo change made if it is not off-screen.");
    o->labelsize(11);
    o->callback((Fl_Callback*)cb_Show);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(385, 486, 40, 16, "Top");
    o->tooltip("Test show_item_top().\nScrolls selected item to the top of the display\n(only"
        " works if scrollbar showing)\nTo use:\n1) open \'500 items\'\n2) select item 0"
        "010\n3) Hit Top/Mid/Bot");
    o->labelsize(11);
    o->callback((Fl_Callback*)cb_Top);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(385, 502, 40, 16, "Mid");
    o->tooltip("Tests show_item_middle().\nScrolls the selected item to the middle of the dis"
        "play\nTo use:\n  1) open \'500 items\'\n  2) select \'item 0010\'\n  3) Hit To"
        "p/Mid/Bot");
    o->labelsize(11);
    o->callback((Fl_Callback*)cb_Mid);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(385, 518, 40, 16, "Bot");
    o->tooltip("Tests show_item_bottom().\nScrolls the selected item to the bottom of the dis"
        "play\nTo use:\n  1) open \'500 items\'\n  2) select \'item 0010\'\n  3) Hit To"
        "p/Mid/Bot");
    o->labelsize(11);
    o->callback((Fl_Callback*)cb_Bot);
    } // Fl_Button* o
    showitem_box->end();
    } // Fl_Group* showitem_box
    { openall_button = new Fl_Button(470, 451, 95, 16, "Open All");
    openall_button->tooltip("Opens all nodes that have children");
    openall_button->labelsize(9);
    openall_button->callback((Fl_Callback*)cb_openall_button);
    } // Fl_Button* openall_button
    { loaddb_button = new Fl_Button(470, 471, 95, 16, "Load Database...");
    loaddb_button->tooltip("Load the contents of an Fl_Preferences database into the tree view");
    loaddb_button->labelsize(9);
    loaddb_button->callback((Fl_Callback*)cb_loaddb_button);
    } // Fl_Button* loaddb_button
    { insertabove_button = new Fl_Button(470, 491, 95, 16, "Insert Above");
    insertabove_button->tooltip("Inserts three items above the selected items");
    insertabove_button->labelsize(9);
    insertabove_button->callback((Fl_Callback*)cb_insertabove_button);
    } // Fl_Button* insertabove_button
    { rebuildtree_button = new Fl_Button(470, 511, 95, 16, "Rebuild Tree");
    rebuildtree_button->tooltip("Rebuilds the tree with defaults");
    rebuildtree_button->labelsize(9);
    rebuildtree_button->callback((Fl_Callback*)cb_rebuildtree_button);
    } // Fl_Button* rebuildtree_button
    { showpathname_button = new Fl_Button(470, 531, 95, 16, "Show Pathname");
    showpathname_button->tooltip("Tests Fl_Tree::item_pathname()\nShow the pathname for the selected item.");
    showpathname_button->labelsize(9);
    showpathname_button->callback((Fl_Callback*)cb_showpathname_button);
    } // Fl_Button* showpathname_button
    { closeall_button = new Fl_Button(570, 451, 95, 16, "Close All");
    closeall_button->tooltip("Closes all nodes that have children\n(doesn\'t affect \'root\')");
    closeall_button->labelsize(9);
    closeall_button->callback((Fl_Callback*)cb_closeall_button);
    } // Fl_Button* closeall_button
    { clearall_button = new Fl_Button(570, 471, 95, 16, "Clear All");
    clearall_button->tooltip("Tests Fl_Tree::clear().\nClears all items");
    clearall_button->labelsize(9);
    clearall_button->callback((Fl_Callback*)cb_clearall_button);
    } // Fl_Button* clearall_button
    { testcallbackflag_button = new Fl_Button(570, 491, 95, 16, "Test Callback Flag");
    testcallbackflag_button->tooltip("Test the \'docallback\' argument can disable callbacks.");
    testcallbackflag_button->labelsize(9);
    testcallbackflag_button->callback((Fl_Callback*)cb_testcallbackflag_button);
    } // Fl_Button* testcallbackflag_button
    { testrootshowself_button = new Fl_Button(570, 511, 95, 16, "Root Show Self");
    testrootshowself_button->tooltip("Test the root->\'show_self() method to show the entire tree on stdout");
    testrootshowself_button->labelsize(9);
    testrootshowself_button->callback((Fl_Callback*)cb_testrootshowself_button);
    } // Fl_Button* testrootshowself_button
    { add20k_button = new Fl_Button(570, 531, 95, 16, "Add 20,000");
    add20k_button->tooltip("Adds 20,000 items to the selected item\'s parent");
    add20k_button->labelsize(9);
    add20k_button->callback((Fl_Callback*)cb_add20k_button);
    } // Fl_Button* add20k_button
    o->end();
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(696, 23, 335, 246, "Selected Items");
    o->tooltip("These controls only affect the selected items. If no items are selected, all "
        "existing items in tree are modified.");
    o->box(FL_GTK_DOWN_BOX);
    o->color(FL_DARK1);
    o->labelsize(12);
    { Fl_Choice* o = selected_labelfont_choice = new Fl_Choice(863, 31, 140, 21, "Fl_Tree_Item::labelfont()");
    selected_labelfont_choice->tooltip("Tests Fl_Tree_Item::labelfont();\nChanges the font for the selected items\'s "
        "labels.\nIf none selected, all are changed.");
    selected_labelfont_choice->down_box(FL_BORDER_BOX);
    selected_labelfont_choice->labelsize(11);
    selected_labelfont_choice->textsize(11);
    selected_labelfont_choice->callback((Fl_Callback*)cb_selected_labelfont_choice);
    selected_labelfont_choice->menu(menu_selected_labelfont_choice);
    o->value((int)tree->item_labelfont());   // get tree's current font, assign to chooser
    } // Fl_Choice* selected_labelfont_choice
    { Fl_Value_Slider* o = selected_labelsize_slider = new Fl_Value_Slider(863, 55, 140, 16, "Fl_Tree_Item::labelsize()");
    selected_labelsize_slider->tooltip("Tests Fl_Tree_Item::labelsize();\nChanges the font size of the selected items"
        "\'s labels.\nIf none selected, all are changed.");
    selected_labelsize_slider->type(1);
    selected_labelsize_slider->color((Fl_Color)46);
    selected_labelsize_slider->selection_color((Fl_Color)1);
    selected_labelsize_slider->labelsize(11);
    selected_labelsize_slider->textsize(12);
    selected_labelsize_slider->callback((Fl_Callback*)cb_selected_labelsize_slider, (void*)(tree));
    selected_labelsize_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->item_labelsize());
    o->range(5.0, 200.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* selected_labelsize_slider
    { Fl_Button* o = all_labelfgcolor_button = new Fl_Button(863, 81, 16, 16, "Fl_Tree_Item::labelfgcolor()");
    all_labelfgcolor_button->tooltip("Sets the Fl_Tree_Item::labelfgcolor() for the selected items. If none selecte"
        "d, all are changed.");
    all_labelfgcolor_button->box(FL_DOWN_BOX);
    all_labelfgcolor_button->labelsize(11);
    all_labelfgcolor_button->callback((Fl_Callback*)cb_all_labelfgcolor_button);
    all_labelfgcolor_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(GetSelectedItemFGColor());
    } // Fl_Button* all_labelfgcolor_button
    { Fl_Button* o = all_labelbgcolor_button = new Fl_Button(863, 99, 16, 16, "Fl_Tree_Item::labelbgcolor()");
    all_labelbgcolor_button->tooltip("Sets the Fl_Tree_Item::labelbgcolor() for the selected items. If none selecte"
        "d, all are changed.");
    all_labelbgcolor_button->box(FL_DOWN_BOX);
    all_labelbgcolor_button->labelsize(11);
    all_labelbgcolor_button->callback((Fl_Callback*)cb_all_labelbgcolor_button);
    all_labelbgcolor_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(GetSelectedItemBGColor());
    } // Fl_Button* all_labelbgcolor_button
    { deactivate_items_toggle = new Fl_Light_Button(758, 134, 100, 16, " Deactivate Items");
    deactivate_items_toggle->tooltip("Toggle the deactivation state of the selected items.\nIf none are selected, a"
        "ll are set.");
    deactivate_items_toggle->selection_color((Fl_Color)1);
    deactivate_items_toggle->labelsize(9);
    deactivate_items_toggle->callback((Fl_Callback*)cb_deactivate_items_toggle);
    } // Fl_Light_Button* deactivate_items_toggle
    { deactivate_tree_toggle = new Fl_Light_Button(758, 154, 100, 16, " Deactivate Tree");
    deactivate_tree_toggle->tooltip("Deactivates the entire tree widget");
    deactivate_tree_toggle->selection_color((Fl_Color)1);
    deactivate_tree_toggle->labelsize(9);
    deactivate_tree_toggle->callback((Fl_Callback*)cb_deactivate_tree_toggle);
    } // Fl_Light_Button* deactivate_tree_toggle
    { bold_toggle = new Fl_Light_Button(758, 174, 100, 16, " Bold Font");
    bold_toggle->tooltip("Toggles bold font for selected items\nIf nothing selected, all are changed");
    bold_toggle->selection_color((Fl_Color)1);
    bold_toggle->labelsize(9);
    bold_toggle->callback((Fl_Callback*)cb_bold_toggle);
    } // Fl_Light_Button* bold_toggle
    { showselected_button = new Fl_Button(864, 134, 95, 16, "Show Selected");
    showselected_button->tooltip("Clears the selected items");
    showselected_button->labelsize(9);
    showselected_button->callback((Fl_Callback*)cb_showselected_button);
    } // Fl_Button* showselected_button
    { clearselected_button = new Fl_Button(864, 154, 95, 16, "Remove Selected");
    clearselected_button->tooltip("Removes the selected items");
    clearselected_button->labelsize(9);
    clearselected_button->callback((Fl_Callback*)cb_clearselected_button);
    } // Fl_Button* clearselected_button
    { swapselected_button = new Fl_Button(864, 174, 95, 16, "Swap Selected");
    swapselected_button->tooltip("Tests the Fl_Tree_Item::swap_children() method\nSwaps two selected items (ite"
        "ms must be siblings)");
    swapselected_button->labelsize(9);
    swapselected_button->callback((Fl_Callback*)cb_swapselected_button);
    } // Fl_Button* swapselected_button
    { selectall_button = new Fl_Button(714, 199, 95, 16, "Select All");
    selectall_button->tooltip("Selects all items in the tree");
    selectall_button->labelsize(9);
    selectall_button->callback((Fl_Callback*)cb_selectall_button);
    } // Fl_Button* selectall_button
    { deselectall_button = new Fl_Button(714, 219, 95, 16, "Deselect All");
    deselectall_button->tooltip("Deselects all items in the tree");
    deselectall_button->labelsize(9);
    deselectall_button->callback((Fl_Callback*)cb_deselectall_button);
    } // Fl_Button* deselectall_button
    { nextselected_button = new Fl_Button(713, 239, 95, 16, "next_selected()");
    nextselected_button->tooltip("Tests the Fl_Tree::next_selected() function");
    nextselected_button->labelsize(9);
    nextselected_button->callback((Fl_Callback*)cb_nextselected_button);
    } // Fl_Button* nextselected_button
    { bbbselect_toggle = new Fl_Light_Button(814, 199, 95, 16, " Select Bbb");
    bbbselect_toggle->tooltip("Toggle selection of just the /Bbb item\n(Not children)");
    bbbselect_toggle->selection_color((Fl_Color)1);
    bbbselect_toggle->labelsize(9);
    bbbselect_toggle->callback((Fl_Callback*)cb_bbbselect_toggle);
    } // Fl_Light_Button* bbbselect_toggle
    { bbbselect2_toggle = new Fl_Light_Button(814, 219, 95, 16, " Select Bbb+");
    bbbselect2_toggle->tooltip("Toggle selection of the /Bbb item and its children");
    bbbselect2_toggle->selection_color((Fl_Color)1);
    bbbselect2_toggle->labelsize(9);
    bbbselect2_toggle->callback((Fl_Callback*)cb_bbbselect2_toggle);
    } // Fl_Light_Button* bbbselect2_toggle
    { bbbchild02select_toggle = new Fl_Light_Button(814, 239, 95, 16, " Toggle child-02");
    bbbchild02select_toggle->tooltip("Toggle the single item \"/Bbb/child-02\" using the item\'s \"pathname\".");
    bbbchild02select_toggle->selection_color((Fl_Color)1);
    bbbchild02select_toggle->labelsize(9);
    bbbchild02select_toggle->callback((Fl_Callback*)cb_bbbchild02select_toggle);
    } // Fl_Light_Button* bbbchild02select_toggle
    { rootselect_toggle = new Fl_Light_Button(914, 199, 100, 16, "Select ROOT");
    rootselect_toggle->tooltip("Toggle selection of the ROOT item");
    rootselect_toggle->selection_color((Fl_Color)1);
    rootselect_toggle->labelsize(9);
    rootselect_toggle->callback((Fl_Callback*)cb_rootselect_toggle);
    } // Fl_Light_Button* rootselect_toggle
    { rootselect2_toggle = new Fl_Light_Button(914, 219, 100, 16, "Select ROOT+");
    rootselect2_toggle->tooltip("Toggle selection of the ROOT item and all children");
    rootselect2_toggle->selection_color((Fl_Color)1);
    rootselect2_toggle->labelsize(9);
    rootselect2_toggle->callback((Fl_Callback*)cb_rootselect2_toggle);
    } // Fl_Light_Button* rootselect2_toggle
    o->end();
    } // Fl_Group* o
    { Fl_Group* o = new Fl_Group(695, 298, 335, 186, "Tree Fonts + Colors");
    o->tooltip("These controls only affect the selected items. If no items are selected, all "
        "existing items in tree are modified.");
    o->box(FL_GTK_DOWN_BOX);
    o->color(FL_DARK1);
    o->labelsize(12);
    { Fl_Choice* o = labelfont_choice = new Fl_Choice(848, 314, 140, 21, "labelfont()");
    labelfont_choice->tooltip("Sets the default font used for new items created. Does NOT affect existing it"
        "ems.");
    labelfont_choice->down_box(FL_BORDER_BOX);
    labelfont_choice->labelsize(12);
    labelfont_choice->textsize(12);
    labelfont_choice->callback((Fl_Callback*)cb_labelfont_choice);
    labelfont_choice->menu(menu_labelfont_choice);
    o->value((int)tree->labelfont());   // get tree's current font, assign to chooser
    } // Fl_Choice* labelfont_choice
    { Fl_Value_Slider* o = labelsize_slider = new Fl_Value_Slider(848, 338, 140, 16, "labelsize()");
    labelsize_slider->tooltip("Sets the font size for the  tree\'s label().\nThis is also the font size that"
        " will be used to draw the items IF their size hasn\'t been set with Fl_Tree_It"
        "em::labelsize() or Fl_Tree::item_labelsize()");
    labelsize_slider->type(1);
    labelsize_slider->color((Fl_Color)46);
    labelsize_slider->selection_color((Fl_Color)1);
    labelsize_slider->labelsize(12);
    labelsize_slider->textsize(12);
    labelsize_slider->callback((Fl_Callback*)cb_labelsize_slider, (void*)(tree));
    labelsize_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value((int)tree->labelsize());
    o->range(1.0, 50.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* labelsize_slider
    { Fl_Choice* o = item_labelfont_choice = new Fl_Choice(848, 358, 140, 21, "Item_labelfont()");
    item_labelfont_choice->tooltip("Sets the default font used for new items created.\n.Also affects any items wh"
        "ose font has NOT specifically been set with item->labelfont().");
    item_labelfont_choice->down_box(FL_BORDER_BOX);
    item_labelfont_choice->labelsize(12);
    item_labelfont_choice->textsize(12);
    item_labelfont_choice->callback((Fl_Callback*)cb_item_labelfont_choice);
    item_labelfont_choice->menu(menu_item_labelfont_choice);
    o->value((int)tree->item_labelfont());
    } // Fl_Choice* item_labelfont_choice
    { Fl_Value_Slider* o = item_labelsize_slider = new Fl_Value_Slider(848, 383, 140, 16, "item_labelsize()");
    item_labelsize_slider->tooltip("Sets the default font size used for new items created.\n.Also affects any ite"
        "ms whose font size has NOT specifically been set with item->labelsize().");
    item_labelsize_slider->type(1);
    item_labelsize_slider->color((Fl_Color)46);
    item_labelsize_slider->selection_color((Fl_Color)1);
    item_labelsize_slider->labelsize(12);
    item_labelsize_slider->textsize(12);
    item_labelsize_slider->callback((Fl_Callback*)cb_item_labelsize_slider, (void*)(tree));
    item_labelsize_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value((int)tree->item_labelsize());
    o->range(1.0, 50.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* item_labelsize_slider
    { Fl_Button* o = labelcolor_button = new Fl_Button(813, 414, 16, 16, "labelcolor()");
    labelcolor_button->tooltip("Changes Fl_Tree::labelcolor().\nThis affects the text color of the widget\'s "
        "label.");
    labelcolor_button->box(FL_DOWN_BOX);
    labelcolor_button->labelsize(11);
    labelcolor_button->callback((Fl_Callback*)cb_labelcolor_button);
    labelcolor_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(tree->labelcolor());
    } // Fl_Button* labelcolor_button
    { Fl_Button* o = color_button = new Fl_Button(813, 433, 16, 16, "color()");
    color_button->tooltip("Changes Fl_Tree::color().\nThis affects the background color of the widget. I"
        "t also affects the bg color of newly created  items *if*  Fl_Tree::item_labelb"
        "gcolor() hasn\'t been changed.");
    color_button->box(FL_DOWN_BOX);
    color_button->labelsize(11);
    color_button->callback((Fl_Callback*)cb_color_button);
    color_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(tree->color());
    } // Fl_Button* color_button
    { Fl_Button* o = selection_color_button = new Fl_Button(813, 452, 16, 16, "selection_color()");
    selection_color_button->tooltip("Sets the Fl_Tree::selection_color().\nThis affects the item\'s colors when th"
        "ey\'re selected.");
    selection_color_button->box(FL_DOWN_BOX);
    selection_color_button->labelsize(11);
    selection_color_button->callback((Fl_Callback*)cb_selection_color_button);
    selection_color_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(tree->selection_color());
    } // Fl_Button* selection_color_button
    { Fl_Button* o = item_labelfgcolor_button = new Fl_Button(973, 414, 16, 16, "item_labelfgcolor()");
    item_labelfgcolor_button->tooltip("Sets the default label fg color for newly created  items.");
    item_labelfgcolor_button->box(FL_DOWN_BOX);
    item_labelfgcolor_button->labelsize(12);
    item_labelfgcolor_button->callback((Fl_Callback*)cb_item_labelfgcolor_button);
    item_labelfgcolor_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    o->color(tree->item_labelfgcolor());
    } // Fl_Button* item_labelfgcolor_button
    { item_labelbgcolor_button = new Fl_Button(973, 433, 16, 16, "item_labelbgcolor()");
    item_labelbgcolor_button->tooltip("Sets the default label bg color for newly created items. When set, this overr"
        "ides the default behavior of using Fl_Tree::color().");
    item_labelbgcolor_button->box(FL_DOWN_BOX);
    item_labelbgcolor_button->labelsize(12);
    item_labelbgcolor_button->callback((Fl_Callback*)cb_item_labelbgcolor_button);
    item_labelbgcolor_button->align(Fl_Align(FL_ALIGN_LEFT_TOP));
    item_labelbgcolor_button->color(tree->item_labelbgcolor());
    } // Fl_Button* item_labelbgcolor_button
    { x_item_labelbgcolor_button = new Fl_Button(993, 433, 16, 16, "X");
    x_item_labelbgcolor_button->tooltip("Make the bgcolor \'transparent\' (0xffffffff)");
    x_item_labelbgcolor_button->labelsize(10);
    x_item_labelbgcolor_button->callback((Fl_Callback*)cb_x_item_labelbgcolor_button);
    x_item_labelbgcolor_button->align(Fl_Align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE));
    } // Fl_Button* x_item_labelbgcolor_button
    o->end();
    } // Fl_Group* o
    { Fl_Value_Slider* o = tree_scrollbar_size_slider = new Fl_Value_Slider(835, 499, 180, 16, "Fl_Tree::scrollbar_size()");
    tree_scrollbar_size_slider->tooltip("Tests Fl_Tree::scrollbar_size() effects on tree clipping.\nThe value is norma"
        "lly 0, which causes Fl_Tree to use the global Fl::scrollbar_size() instead.");
    tree_scrollbar_size_slider->type(1);
    tree_scrollbar_size_slider->color((Fl_Color)46);
    tree_scrollbar_size_slider->selection_color((Fl_Color)1);
    tree_scrollbar_size_slider->labelsize(11);
    tree_scrollbar_size_slider->textsize(9);
    tree_scrollbar_size_slider->callback((Fl_Callback*)cb_tree_scrollbar_size_slider);
    tree_scrollbar_size_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(tree->scrollbar_size());
    o->range(0.0, 30.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* tree_scrollbar_size_slider
    { Fl_Value_Slider* o = scrollbar_size_slider = new Fl_Value_Slider(835, 519, 180, 16, "Fl::scrollbar_size()");
    scrollbar_size_slider->tooltip("Tests Fl::scrollbar_size() effects on tree clipping");
    scrollbar_size_slider->type(1);
    scrollbar_size_slider->color((Fl_Color)46);
    scrollbar_size_slider->selection_color((Fl_Color)1);
    scrollbar_size_slider->labelsize(11);
    scrollbar_size_slider->textsize(9);
    scrollbar_size_slider->callback((Fl_Callback*)cb_scrollbar_size_slider);
    scrollbar_size_slider->align(Fl_Align(FL_ALIGN_LEFT));
    o->value(Fl::scrollbar_size());
    o->range(5.0, 30.0);
    o->step(1.0);
    o->color(46); o->selection_color(FL_RED);
    } // Fl_Value_Slider* scrollbar_size_slider
    { testsuggs_button = new Fl_Button(935, 545, 95, 16, "Test Suggestions");
    testsuggs_button->tooltip("Suggestions on how to do tests");
    testsuggs_button->labelsize(9);
    testsuggs_button->callback((Fl_Callback*)cb_testsuggs_button);
    } // Fl_Button* testsuggs_button
    o->resizable(0);
    o->end();
    } // Fl_Group* o
    { resizer_box = new Fl_Box(0, 263, 15, 14);
    } // Fl_Box* resizer_box
    { tty = new Fl_Terminal(16, 571, 1014, 149);
    } // Fl_Terminal* tty
    window->end();
  } // Fl_Double_Window* window
  // Initialize Tree
    tree->root_label("ROOT");
    tree->item_reselect_mode(FL_TREE_SELECTABLE_ALWAYS);
    RebuildTree();
    /*tree->show_self();*/
    // FLTK stuff

    //Fl::scheme("gtk+");

    Fl_Tooltip::size(10);           // small font for tooltips

    window->resizable(tree);
    window->size_range(window->w(), window->h(), 0, 0);

    if (tree->when() == FL_WHEN_CHANGED) whenmode_chooser->value(0);
    else if (tree->when() == FL_WHEN_RELEASE) whenmode_chooser->value(1);
    else if (tree->when() == FL_WHEN_NEVER) whenmode_chooser->value(2);
    window->show(argc, argv);
    Fl::run();

#ifndef _DEBUG
    /* Cleanup allocated memory for argv */
    for (int i = 0; i < argc; ++i) {
        free((void*)argv[i]);
    }
    free((void*)argv);
#endif
    return(0);
}